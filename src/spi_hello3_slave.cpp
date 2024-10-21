#include <Arduino.h>
#include <SPI.h>
#include <esp_camera.h>
#include <esp_log.h>

static const char* TAG = "example:spi_hello3_slave";

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1 // software reset will be performed
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27
#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

#define SPI_MOSI 12
#define SPI_MISO 13
#define SPI_SCK 15
#define SPI_SS 14

#define QQVGA_WIDTH 160
#define QQVGA_HEIGHT 120
#define QQVGA_FRAME_SIZE (QQVGA_WIDTH * QQVGA_HEIGHT)

uint8_t* frame_buffer;

void setup_camera();
void setup_spi_slave();
void task_camera_capture(void* parameter);
void task_spi_communication(void* parameter);

void setup() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    
    Serial.begin(115200);
    ESP_LOGI(TAG, "Starting SPI slave setup");

    setup_spi_slave();

    // allocate frame buffer
    frame_buffer = (uint8_t*)malloc(QQVGA_FRAME_SIZE);
    if (frame_buffer == NULL) {
        ESP_LOGE(TAG, "Failed to allocate frame buffer");
        return;
    }
    ESP_LOGI(TAG, "Frame buffer allocated successfully");

    // tasks for each core
    xTaskCreatePinnedToCore(task_camera_capture, "Camera Capture", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(task_spi_communication, "SPI Communication", 4096, NULL, 1, NULL, 1);

    ESP_LOGI(TAG, "Tasks created successfully");
}

void loop() {
    // Empty loop as we're using FreeRTOS tasks
}

void setup_camera() {
    camera_config_t config = {
        .pin_pwdn     = CAM_PIN_PWDN,
        .pin_reset    = CAM_PIN_RESET,
        .pin_xclk     = CAM_PIN_XCLK,
        .pin_sccb_sda = CAM_PIN_SIOD,
        .pin_sccb_scl = CAM_PIN_SIOC,
        .pin_d7       = CAM_PIN_D7,
        .pin_d6       = CAM_PIN_D6,
        .pin_d5       = CAM_PIN_D5,
        .pin_d4       = CAM_PIN_D4,
        .pin_d3       = CAM_PIN_D3,
        .pin_d2       = CAM_PIN_D2,
        .pin_d1       = CAM_PIN_D1,
        .pin_d0       = CAM_PIN_D0,
        .pin_vsync    = CAM_PIN_VSYNC,
        .pin_href     = CAM_PIN_HREF,
        .pin_pclk     = CAM_PIN_PCLK,
        .xclk_freq_hz = 20000000,
        .ledc_timer   = LEDC_TIMER_0,
        .ledc_channel = LEDC_CHANNEL_0,
        .pixel_format = PIXFORMAT_GRAYSCALE,
        .frame_size   = FRAMESIZE_QQVGA,
        // .frame_size   = FRAMESIZE_96X96,
        .jpeg_quality = 12,
        .fb_count     = 1,
        // .fb_location  = CAMERA_FB_IN_DRAM,
        .fb_location = CAMERA_FB_IN_PSRAM,
        // .grab_mode   = CAMERA_GRAB_WHEN_EMPTY,
        .grab_mode = CAMERA_GRAB_LATEST,
    };

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x", err);
        return;
    }
}

void setup_spi_slave() {
    ESP_LOGI(TAG, "Setting up SPI slave");
    pinMode(SPI_SS, INPUT);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SPI_SS);
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    ESP_LOGI(TAG, "SPI slave setup completed");
}

void task_camera_capture(void* parameter) {
    ESP_LOGI(TAG, "Starting camera capture task");
    while (true) {
        camera_fb_t* fb = esp_camera_fb_get();
        if (!fb) {
            ESP_LOGE(TAG, "Camera capture failed");
            continue;
        }

        memcpy(frame_buffer, fb->buf, QQVGA_FRAME_SIZE);

        esp_camera_fb_return(fb);
        ESP_LOGD(TAG, "Frame captured and stored in buffer");
        vTaskDelay(pdMS_TO_TICKS(100)); // capture at 10 FPS
    }
}

void task_spi_communication(void* parameter) {
    ESP_LOGI(TAG, "Starting SPI communication task");
    while (true) {
        if (digitalRead(SPI_SS) == LOW) {
            // master is requesting data
            ESP_LOGD(TAG, "SPI transfer started");
            SPI.transferBytes(frame_buffer, NULL, QQVGA_FRAME_SIZE);
            ESP_LOGD(TAG, "SPI transfer completed");
        }
        vTaskDelay(1); // delay to prevent tight loop
    }
}