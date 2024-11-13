#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include <Arduino.h>
#include <WiFi.h>
#include <driver/spi_master.h>
#include <esp_http_server.h>

const char* ssid     = "TAMU_IoT";
const char* password = "22222222";

// SPI pins for master
#define GPIO_MOSI 13
#define GPIO_MISO 12
#define GPIO_SCLK 14
#define GPIO_CS 15

static const char* TAG = "example:spi_bitmap_master";

// SPI handle
spi_device_handle_t spi_handle;

// buffer for received JPEG
uint8_t*        jpeg_buffer      = nullptr;
size_t          jpeg_buffer_size = 160 * 120 * 2; // worst case JPEG size (is it??)
volatile size_t jpeg_len         = 0;

// transaction buffer for receiving JPEG size
union {
    uint32_t size;
    uint8_t  bytes[4];
} jpeg_size_buffer;

// FPS calculation
uint32_t frame_count    = 0;
uint32_t last_fps_check = 0;
float    current_fps    = 0.0;

void update_fps() {
    frame_count++;

    uint32_t current_time = millis();
    uint32_t elapsed_time = current_time - last_fps_check;

    if (elapsed_time >= 1000) { // Calculate FPS every second
        current_fps    = frame_count * 1000.0f / elapsed_time;
        frame_count    = 0;
        last_fps_check = current_time;
        ESP_LOGI(TAG, "Current FPS: %.1f", current_fps);
    }
}

// HTTP streaming
static const char* _STREAM_CONTENT_TYPE = "multipart/x-mixed-replace;boundary=frame";
static const char* _STREAM_BOUNDARY     = "\r\n--frame\r\n";
static const char* _STREAM_PART         = "Content-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n";

static esp_err_t stream_handler(httpd_req_t* req) {
    esp_err_t res = ESP_OK;

    res = httpd_resp_set_type(req, _STREAM_CONTENT_TYPE);
    if (res != ESP_OK)
        return res;

    while (1) {
        spi_transaction_t t;
        memset(&t, 0, sizeof(t));

        // IDEA:
        // first transaction: receive JPEG size
        t.length    = 32; // 4 bytes * 8 bits
        t.rx_buffer = jpeg_size_buffer.bytes;

        esp_err_t ret = spi_device_transmit(spi_handle, &t);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to receive JPEG size");
            continue;
        }

        jpeg_len = jpeg_size_buffer.size;
        if (jpeg_len == 0 || jpeg_len > jpeg_buffer_size) {
            ESP_LOGE(TAG, "Invalid JPEG size: %d", jpeg_len);
            continue;
        }

        // IDEA:
        // second transaction: receive JPEG data
        memset(&t, 0, sizeof(t));
        t.length    = jpeg_len * 8;
        t.rx_buffer = jpeg_buffer;

        ret = spi_device_transmit(spi_handle, &t);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to receive JPEG data");
            continue;
        }

        // send frame through HTTP
        httpd_resp_send_chunk(req, _STREAM_BOUNDARY, strlen(_STREAM_BOUNDARY));

        char   part_buf[128];
        size_t part_buf_len = snprintf(part_buf, 64, _STREAM_PART, jpeg_len);
        httpd_resp_send_chunk(req, part_buf, part_buf_len);

        httpd_resp_send_chunk(req, (char*)jpeg_buffer, jpeg_len);

        if (res != ESP_OK) {
            break;
        }

        update_fps();
        delay(1);
    }

    return res;
}

void setup() {
    // disable brownout detector
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    while (!Serial) {
        ;
    }

    // IMPORTANT!!!!!!: allocate DMA-capable memory for JPEG buffer
    jpeg_buffer = (uint8_t*)heap_caps_aligned_alloc(4, jpeg_buffer_size,
                                                    MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    if (!jpeg_buffer) {
        ESP_LOGE(TAG, "Failed to allocate JPEG buffer");
        return;
    }

    // configuration for the SPI bus
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num      = GPIO_MOSI;
    buscfg.miso_io_num      = GPIO_MISO;
    buscfg.sclk_io_num      = GPIO_SCLK;
    buscfg.quadwp_io_num    = -1;
    buscfg.quadhd_io_num    = -1;
    buscfg.max_transfer_sz  = jpeg_buffer_size;

    // configuration for the SPI device
    spi_device_interface_config_t devcfg = {};
    devcfg.command_bits                  = 0;
    devcfg.address_bits                  = 0;
    devcfg.dummy_bits                    = 0;
    devcfg.clock_speed_hz                = 5000000;
    devcfg.duty_cycle_pos                = 32; // 128 = 50% duty cycle
    devcfg.mode                          = 0;
    devcfg.spics_io_num                  = GPIO_CS;
    devcfg.cs_ena_posttrans              = 3; // keep the CS low 3 cycles after transaction
    devcfg.queue_size                    = 3;

    // initialize SPI bus and add device
    esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus");
        return;
    }

    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device");
        return;
    }

    ESP_LOGI(TAG, "SPI Master initialized successfully");

    // connect to WiFi
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // start HTTP server
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.stack_size     = 8192;

    httpd_handle_t stream_httpd = NULL;
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_uri_t stream_uri = {
            .uri = "/stream", .method = HTTP_GET, .handler = stream_handler, .user_ctx = NULL};
        httpd_register_uri_handler(stream_httpd, &stream_uri);
        ESP_LOGI(TAG, "Stream ready on http://%s/stream", WiFi.localIP().toString().c_str());
    }
}

void loop() {
    delay(1);
}