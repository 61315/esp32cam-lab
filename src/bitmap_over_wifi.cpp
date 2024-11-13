#include <Arduino.h>
#include <WiFi.h>
#include <esp_http_server.h>
#include <esp_camera.h>
#include "Adafruit_GFX.h"
#include "soc/soc.h"           // for brownout disable
#include "soc/rtc_cntl_reg.h"  // for brownout disable

// wifi credentials
const char* ssid     = "TAMU_IoT";
const char* password = "22222222";

// bitmap dimensions (qqvga)
#define BITMAP_WIDTH 160
#define BITMAP_HEIGHT 120

// bitmap buffer
uint8_t bitmap[BITMAP_WIDTH * BITMAP_HEIGHT];

// jpeg conversion buffer
uint8_t* jpeg_buffer = nullptr;
size_t jpeg_len = 0;

// frame counter
uint32_t frame_count = 0;
uint32_t last_fps_check = 0;
float current_fps = 0.0;

// gfx class for grayscale bitmap
class gray_bitmap : public Adafruit_GFX {
public:
    gray_bitmap(uint8_t* buffer, int16_t w, int16_t h) 
        : Adafruit_GFX(w, h), buffer_(buffer) {}

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (x < 0 || x >= _width || y < 0 || y >= _height) return;
        buffer_[y * _width + x] = color & 0xFF;
    }

    uint8_t* get_buffer() { return buffer_; }

private:
    uint8_t* buffer_;
};

// global gfx object
gray_bitmap* gfx = nullptr;

// moving text position
int text_x = 0;
int text_y = 10;
int direction_x = 1;

// http response header
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=frame";
static const char* _STREAM_BOUNDARY = "\r\n--frame\r\n";
static const char* _STREAM_PART = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

void update_fps() {
    frame_count++;
    
    uint32_t current_time = millis();
    uint32_t time_diff = current_time - last_fps_check;
    
    if (time_diff >= 1000) {  // calculate FPS every second
        current_fps = frame_count * 1000.0 / time_diff;
        frame_count = 0;
        last_fps_check = current_time;
    }
}

// handle stream request
static esp_err_t stream_handler(httpd_req_t *req) {
    esp_err_t res = ESP_OK;
    
    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK) return res;
    
    while (1) {
        uint32_t now = millis();
        
        // clear and draw on bitmap
        gfx->fillScreen(0);
        
        // draw animated border
        uint8_t border_color = (now / 500) % 255;
        gfx->drawRect(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, border_color);
        
        // draw moving text (main title)
        gfx->setTextColor(255);
        gfx->setTextSize(2);
        gfx->setCursor(text_x, text_y);
        gfx->print("Gig 'em!");

        // draw system info (single column with padding)
        gfx->setTextSize(1);
        const int left_margin = 3;  // pixels from left edge
        
        // draw uptime with milliseconds
        gfx->setCursor(left_margin, BITMAP_HEIGHT - 84);
        uint32_t uptime_sec = now / 1000;
        uint32_t uptime_min = uptime_sec / 60;
        uint32_t uptime_hr = uptime_min / 60;
        uint32_t uptime_ms = now % 1000;
        gfx->printf("up: %02d:%02d:%02d.%03d", uptime_hr, uptime_min % 60, uptime_sec % 60, uptime_ms);
        
        // draw fps
        gfx->setCursor(left_margin, BITMAP_HEIGHT - 72);
        gfx->printf("fps: %.1f", current_fps);
        
        // draw frame number
        gfx->setCursor(left_margin, BITMAP_HEIGHT - 60);
        gfx->printf("frame: %d", frame_count);

        // draw free heap
        gfx->setCursor(left_margin, BITMAP_HEIGHT - 48);
        gfx->printf("heap: %d kb", ESP.getFreeHeap() / 1024);
        
        // draw image dimensions
        gfx->setCursor(left_margin, BITMAP_HEIGHT - 36);
        gfx->printf("size: %dx%d", BITMAP_WIDTH, BITMAP_HEIGHT);
        
        // draw format info
        gfx->setCursor(left_margin, BITMAP_HEIGHT - 24);
        gfx->printf("fmt: gray->jpg");
        
        // draw buffer sizes
        gfx->setCursor(left_margin, BITMAP_HEIGHT - 12);
        gfx->printf("raw: %d b, jpg: %d b", BITMAP_WIDTH * BITMAP_HEIGHT, jpeg_len);

        // draw diagonal line
        gfx->drawLine(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, 128);
        
        // draw moving circle
        int circle_x = BITMAP_WIDTH/2 + sin(now/1000.0) * 30;
        int circle_y = BITMAP_HEIGHT/2 + cos(now/1000.0) * 20;
        gfx->fillCircle(circle_x, circle_y, 10, 200);

        // update text position
        text_x += direction_x * 2;
        if (text_x >= BITMAP_WIDTH - 50 || text_x <= 0) {
            direction_x *= -1;
        }

        // convert grayscale to jpeg
        bool jpeg_converted = fmt2jpg(bitmap, BITMAP_WIDTH * BITMAP_HEIGHT, 
                                    BITMAP_WIDTH, BITMAP_HEIGHT, 
                                    PIXFORMAT_GRAYSCALE, 20, 
                                    &jpeg_buffer, &jpeg_len);
                                    
        if (!jpeg_converted) {
            Serial.println("jpeg conversion failed");
            continue;
        }

        // send frame
        httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        
        char part_buf[128];
        size_t part_buf_len = snprintf(part_buf, 64, _STREAM_PART, jpeg_len);
        httpd_resp_send_chunk(req, part_buf, part_buf_len);
        
        httpd_resp_send_chunk(req, (char *)jpeg_buffer, jpeg_len);
        
        // free jpeg buffer
        free(jpeg_buffer);
        jpeg_buffer = nullptr;
        
        if (res != ESP_OK) {
            break;
        }

        update_fps();
        
        // control frame rate
        delay(50);  // target ~20fps
    }
    
    return res;
}

void setup() {
    // disable brownout detector
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
    
    Serial.begin(115200);
    
    // initialize gfx
    gfx = new gray_bitmap(bitmap, BITMAP_WIDTH, BITMAP_HEIGHT);
    
    // connect to wifi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("wifi connected");
    Serial.println(WiFi.localIP());
    
    // start http server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t stream_httpd = NULL;
    
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_uri_t stream_uri = {
            .uri = "/stream",
            .method = HTTP_GET,
            .handler = stream_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(stream_httpd, &stream_uri);
        Serial.printf("stream ready on http://%s/stream\n", WiFi.localIP().toString().c_str());
    }
}

void loop() {
    delay(1);
}