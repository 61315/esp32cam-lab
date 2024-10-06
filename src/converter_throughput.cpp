#include <Arduino.h>
#include <esp_camera.h>
#include <esp_heap_caps.h>
#include <esp_timer.h>
#include <img_converters.h>

extern const uint8_t qqvga_gray_bin_start[] asm("_binary_assets_qqvga_gray_bin_start");
extern const uint8_t qqvga_gray_bin_end[] asm("_binary_assets_qqvga_gray_bin_end");
extern const uint8_t qvga_gray_bin_start[] asm("_binary_assets_qvga_gray_bin_start");
extern const uint8_t qvga_gray_bin_end[] asm("_binary_assets_qvga_gray_bin_end");

#define QQVGA_WIDTH 160
#define QQVGA_HEIGHT 120
#define QVGA_WIDTH 320
#define QVGA_HEIGHT 240

// (0: QQVGA, 1: QVGA)
#define RESOLUTION 0

uint8_t* temp_buf = nullptr;
size_t   img_len  = 0;
int      img_width, img_height;

uint32_t frame_count     = 0;
uint64_t last_fps_update = 0;

void print_memory_info() {
    Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
    Serial.printf("Largest free block: %u bytes\n",
                  heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    Serial.printf("Free PSRAM: %u bytes\n", ESP.getFreePsram());
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // wail until port open
    }

    if (RESOLUTION == 0) {
        Serial.println("ESP32-CAM QQVGA Grayscale to JPEG Conversion Test");
        img_width  = QQVGA_WIDTH;
        img_height = QQVGA_HEIGHT;
        img_len    = qqvga_gray_bin_end - qqvga_gray_bin_start;
    } else {
        Serial.println("ESP32-CAM QVGA Grayscale to JPEG Conversion Test");
        img_width  = QVGA_WIDTH;
        img_height = QVGA_HEIGHT;
        img_len    = qvga_gray_bin_end - qvga_gray_bin_start;
    }

    temp_buf = (uint8_t*)heap_caps_malloc(img_len, MALLOC_CAP_SPIRAM);
    if (!temp_buf) {
        Serial.println("Failed to allocate memory for temporary buffer");
        print_memory_info();
        return;
    }

    if (RESOLUTION == 0) {
        memcpy(temp_buf, qqvga_gray_bin_start, img_len);
    } else {
        memcpy(temp_buf, qvga_gray_bin_start, img_len);
    }

    last_fps_update = esp_timer_get_time();

    print_memory_info();
}

void loop() {
    uint8_t* jpg_buf     = NULL;
    size_t   jpg_buf_len = 0;

    // gray2jpg
    bool converted = fmt2jpg(temp_buf, img_len, img_width, img_height, PIXFORMAT_GRAYSCALE, 80,
                             &jpg_buf, &jpg_buf_len);
    if (!converted) {
        Serial.println("Conversion failed!");
        print_memory_info();
        delay(1000);
        return;
    }

    frame_count++;

    uint64_t now = esp_timer_get_time();

    if (now - last_fps_update > 1000000) {
        float fps = frame_count / ((now - last_fps_update) / 1000000.0);
        Serial.printf("Current FPS: %.2f, JPEG size: %u bytes\n", fps, jpg_buf_len);
        print_memory_info();

        frame_count     = 0;
        last_fps_update = now;
    }

    free(jpg_buf);

    heap_caps_check_integrity_all(true);
}