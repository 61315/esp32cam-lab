#define BOARD_ESP32CAM_AITHINKER
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <string.h>
#include <sys/param.h>

// support IDF 5.x
#ifndef portTICK_RATE_MS
#define portTICK_RATE_MS portTICK_PERIOD_MS
#endif
#include "esp_camera.h"

// ESP32Cam (AiThinker) PIN Map
#ifdef BOARD_ESP32CAM_AITHINKER
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
#endif

static const char* TAG = "example:take_picture";

static camera_config_t camera_config = {
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
    .jpeg_quality = 6,
    .fb_count     = 3,
    // .fb_location  = CAMERA_FB_IN_DRAM,
    .fb_location  = CAMERA_FB_IN_PSRAM,
    .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
    // .grab_mode = CAMERA_GRAB_LATEST,
};

// typedef enum {
//     PIXFORMAT_RGB565,    // 2BPP/RGB565
//     PIXFORMAT_YUV422,    // 2BPP/YUV422
//     PIXFORMAT_YUV420,    // 1.5BPP/YUV420
//     PIXFORMAT_GRAYSCALE, // 1BPP/GRAYSCALE
//     PIXFORMAT_JPEG,      // JPEG/COMPRESSED
//     PIXFORMAT_RGB888,    // 3BPP/RGB888
//     PIXFORMAT_RAW,       // RAW
//     PIXFORMAT_RGB444,    // 3BP2P/RGB444
//     PIXFORMAT_RGB555,    // 3BP2P/RGB555
// } pixformat_t;

static esp_err_t init_camera(void) {
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }
    return ESP_OK;
}

#define RECENT_IMAGE_COUNT 5

uint32_t image_sizes[RECENT_IMAGE_COUNT] = {0};
int      image_index                     = 0;

void setup(void) {
    vTaskDelay(2000 / portTICK_RATE_MS);
    Serial.begin(115200);

    if (ESP_OK != init_camera()) {
        return;
    }

    uint32_t start_time       = millis();
    uint32_t frame_count      = 0;
    uint32_t last_report_time = start_time;

    while (1) {
        // ESP_LOGI(TAG, "Taking picture...");
        camera_fb_t* pic = esp_camera_fb_get();

        if (pic) {
            frame_count++;

            // Store the image size in the circular buffer
            image_sizes[image_index] = pic->len;
            image_index              = (image_index + 1) % RECENT_IMAGE_COUNT;

            esp_camera_fb_return(pic);

            uint32_t current_time = millis();
            uint32_t elapsed_time = current_time - last_report_time;

            // Report every second
            if (elapsed_time >= 1000) {
                float fps      = frame_count * 1000.0f / elapsed_time;
                float avg_size = 0;
                for (int i = 0; i < RECENT_IMAGE_COUNT; i++) {
                    avg_size += image_sizes[i];
                }
                avg_size /= RECENT_IMAGE_COUNT;

                Serial.printf("Throughput: %.2f FPS, Avg size of last %d images: %.2f bytes\n", fps,
                              RECENT_IMAGE_COUNT, avg_size);

                // Reset counters
                frame_count      = 0;
                last_report_time = current_time;
            }
        } else {
            ESP_LOGE(TAG, "Camera capture failed");
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    }
}

void loop() {
    // Empty as we're using the setup() function for the main loop
}