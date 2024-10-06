#include "Arduino.h"
#include "esp_camera.h"
#include "esp_http_server.h"
#include "esp_timer.h"
#include "fb_gfx.h"
#include "img_converters.h"
#include "soc/rtc_cntl_reg.h" // disable brownout problems
#include "soc/soc.h"          // disable brownout problems
#include <WiFi.h>

// Replace with your network credentials
const char* ssid     = "TAMU_IoT";
const char* password = "";

#define PART_BOUNDARY "123456789000000000000987654321"

#define CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define S1Tx 12
#define S1Rx 13
#define S2Tx 15
#define S2Rx 14

static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=" PART_BOUNDARY;
static const char* _STREAM_BOUNDARY     = "\r\n--" PART_BOUNDARY "\r\n";
static const char* _STREAM_PART         = "Content-Type: image/jpeg\r\nContent-Length: "
                                          "%u\r\nX-Timestamp: %d.%06d\r\n\r\n";

httpd_handle_t stream_httpd  = NULL;
httpd_handle_t stream_httpd1 = NULL;
httpd_handle_t stream_httpd2 = NULL;

void esp_serial_fb_return(camera_fb_t* fb) {
    free(fb->buf);
    free(fb);
}

camera_fb_t* esp_serial_fb_get(uint8_t serial) {
    char*          buffer        = (char*)malloc(sizeof(camera_fb_t));
    camera_fb_t*   fb            = (camera_fb_t*)buffer;
    HardwareSerial currentSerial = NULL;
    switch (serial) {
    case 1:
        currentSerial = Serial1;
        break;
    case 2:
        currentSerial = Serial2;
        break;
    }
    while (currentSerial.available())
        currentSerial.read();
    currentSerial.print("GETINFO\n");
    delay(1);
    int rb = currentSerial.readBytes(buffer, sizeof(camera_fb_t));
    Serial.print("..GOTRaw : ");
    for (int i = 0; i < sizeof(camera_fb_t); i++)
        Serial.printf("%02x ", *(buffer + i));
    Serial.println();
    Serial.printf("..GOT :length=%u width:%u height:%u format:%u\n", fb->len, fb->width, fb->height,
                  fb->format);
    if (!rb || fb->width > 1920) {
        free(fb);
        return NULL;
    }
    fb->buf    = (uint8_t*)malloc(fb->len);
    int remain = fb->len;
    currentSerial.print("GETFB\n");
    for (size_t i = 0; remain > 0; i += 128) {
        rb = currentSerial.readBytes(fb->buf + i, min(remain, 128));
        Serial.printf("reading... RB:%d\n", rb);
        remain -= 128;
        currentSerial.print("GETFB\n");
    }
    Serial.printf("Read done!\n");

    if (!rb) {
        esp_serial_fb_return(fb);
        return NULL;
    }
    return fb;
}
static esp_err_t stream_handler(httpd_req_t* req, uint8_t selector) {
    long           last_request_time = 0;
    camera_fb_t*   fb                = NULL;
    struct timeval _timestamp;

    esp_err_t res = ESP_OK;

    size_t   _jpg_buf_len = 0;
    uint8_t* _jpg_buf     = NULL;

    char* part_buf[256];

    static int64_t last_frame = 0;
    if (!last_frame)
        last_frame = esp_timer_get_time();

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
        return res;

    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "X-Framerate", "60");

    while (true) {
        fb = (!selector) ? esp_camera_fb_get() : esp_serial_fb_get(selector);
        if (selector)
            Serial.print("FB GOT!\n");
        if (!fb) {
            log_e("Camera capture failed with response: %s", esp_err_to_name(res));
            res = ESP_FAIL;
        } else {
            _timestamp.tv_sec  = fb->timestamp.tv_sec;
            _timestamp.tv_usec = fb->timestamp.tv_usec;
            _jpg_buf_len       = fb->len;
            _jpg_buf           = fb->buf;
        }
        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));
        if (res == ESP_OK) {
            if (selector)
                Serial.print("Sending HTTP\n");
            size_t hlen = snprintf((char*)part_buf, 128, _STREAM_PART, _jpg_buf_len,
                                   _timestamp.tv_sec, _timestamp.tv_usec);
            res         = httpd_resp_send_chunk(req, (const char*)part_buf, hlen);
        }
        if (res == ESP_OK)
            res = httpd_resp_send_chunk(req, (const char*)_jpg_buf, _jpg_buf_len);
        if (fb) {
            !selector ? esp_camera_fb_return(fb) : esp_serial_fb_return(fb);
            fb       = NULL;
            _jpg_buf = NULL;
        } else if (_jpg_buf) {
            free(_jpg_buf);
            _jpg_buf = NULL;
        }
        if (res != ESP_OK)
            break;
        long request_end  = millis();
        long latency      = (request_end - last_request_time);
        last_request_time = request_end;
        // log_d("Size: %uKB, Time: %ums (%ifps)\n", _jpg_buf_len / 1024, latency,
        // 1000 / latency);
    }
    last_frame = 0;
    return res;
}

static esp_err_t stream_handler(httpd_req_t* req) {
    return stream_handler(req, 0);
}
static esp_err_t stream_handler_left(httpd_req_t* req) {
    return stream_handler(req, 1);
}
static esp_err_t stream_handler_right(httpd_req_t* req) {
    return stream_handler(req, 2);
}

void startCameraServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port    = 80;
    httpd_uri_t index_uri = {
        .uri = "/", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL};
    httpd_uri_t left_uri = {
        .uri = "/", .method = HTTP_GET, .handler = stream_handler_left, .user_ctx = NULL};
    httpd_uri_t right_uri = {
        .uri = "/", .method = HTTP_GET, .handler = stream_handler_right, .user_ctx = NULL};
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &index_uri);
    }
    config.server_port++;
    config.ctrl_port++;
    if (httpd_start(&stream_httpd1, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd1, &left_uri);
    }
    config.server_port++;
    config.ctrl_port++;
    if (httpd_start(&stream_httpd2, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd2, &right_uri);
    }
}

void setup() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

    Serial.begin(115200);
    Serial1.setPins(S1Rx, S1Tx);
    Serial1.setRxBufferSize(8192);
    Serial1.begin(921600);
    Serial2.setPins(S2Rx, S2Tx);
    Serial2.setRxBufferSize(8192);
    Serial2.begin(921600);
    Serial.setDebugOutput(true);
    // esp_log_level_set("Boronare>", ESP_LOG_INFO);

    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = Y2_GPIO_NUM;
    config.pin_d1       = Y3_GPIO_NUM;
    config.pin_d2       = Y4_GPIO_NUM;
    config.pin_d3       = Y5_GPIO_NUM;
    config.pin_d4       = Y6_GPIO_NUM;
    config.pin_d5       = Y7_GPIO_NUM;
    config.pin_d6       = Y8_GPIO_NUM;
    config.pin_d7       = Y9_GPIO_NUM;
    config.pin_xclk     = XCLK_GPIO_NUM;
    config.pin_pclk     = PCLK_GPIO_NUM;
    config.pin_vsync    = VSYNC_GPIO_NUM;
    config.pin_href     = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn     = PWDN_GPIO_NUM;
    config.pin_reset    = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size   = FRAMESIZE_QQVGA;
    config.jpeg_quality = 8;
    config.fb_count     = 3;
    config.fb_location  = CAMERA_FB_IN_DRAM;

    // Camera init
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        //    return;
    }
    // Wi-Fi connection
    Serial.print("WiFiConnecting...\n");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");

    Serial.print("Camera Stream Ready! Go to: http://");
    Serial.println(WiFi.localIP());

    // Start streaming web server
    startCameraServer();
}

void loop() {}