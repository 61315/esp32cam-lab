#include "driver/rtc_io.h"
#include "esp32-hal-cpu.h"
#include "esp_bt.h"
#include "esp_camera.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include <Arduino.h>

static const char* TAG = "example:system_info";

void printSystemInfo() {
    // Chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    ESP_LOGI(TAG, "ESP32 Chip Information:");
    ESP_LOGI(TAG, "  - Model: %s", ESP.getChipModel());
    ESP_LOGI(TAG, "  - Cores: %d", chip_info.cores);
    ESP_LOGI(TAG, "  - Features: WiFi%s%s", (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(TAG, "  - Silicon revision: %d", chip_info.revision);

    // Flash memory
    uint32_t flash_size = spi_flash_get_chip_size();
    ESP_LOGI(TAG, "Flash Memory: %dMB %s", flash_size / (1024 * 1024),
             (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    // CPU information
    ESP_LOGI(TAG, "CPU Frequency: %dMHz", ESP.getCpuFreqMHz());

    // RAM information
    ESP_LOGI(TAG, "SRAM Size: %d KB", ESP.getHeapSize() / 1024);
    ESP_LOGI(TAG, "Available SRAM: %d KB", ESP.getFreeHeap() / 1024);

    // MAC address
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    ESP_LOGI(TAG, "MAC Address: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3],
             mac[4], mac[5]);

    // Camera information
    // camera_config_t config;
    // esp_err_t       err = esp_camera_init(&config);
    // if (err != ESP_OK) {
    //     ESP_LOGE(TAG, "Camera initialization failed with error 0x%x", err);
    // } else {
    //     sensor_t* s = esp_camera_sensor_get();
    //     if (s) {
    //         ESP_LOGI(TAG, "Camera Sensor ID: 0x%x", s->id);
    //         ESP_LOGI(TAG, "Camera Resolution: %d x %d", s->status.framesize,
    //         s->status.framesize);
    //     } else {
    //         ESP_LOGE(TAG, "Failed to get camera sensor information");
    //     }
    // }

    // WiFi mode
    wifi_mode_t wifi_mode;
    if (esp_wifi_get_mode(&wifi_mode) == ESP_OK) {
        ESP_LOGI(TAG, "WiFi Mode: %s",
                 wifi_mode == WIFI_MODE_NULL    ? "Null"
                 : wifi_mode == WIFI_MODE_STA   ? "Station"
                 : wifi_mode == WIFI_MODE_AP    ? "Access Point"
                 : wifi_mode == WIFI_MODE_APSTA ? "Access Point + Station"
                                                : "Unknown");
    } else {
        ESP_LOGE(TAG, "Failed to get WiFi mode");
    }

    // Battery voltage (if measurable through ADC)
    // Note: This part may need adjustment based on your hardware configuration
    int   batteryVoltage = analogRead(35); // Assuming battery voltage is connected to GPIO 35
    float voltage        = batteryVoltage / 4095.0 * 3.3 * 2; // Assuming voltage divider is used
    ESP_LOGI(TAG, "Battery Voltage: %.2fV", voltage);

    // System uptime
    ESP_LOGI(TAG, "System Uptime: %llu seconds", esp_timer_get_time() / 1000000ULL);
}

void setup() {
    esp_log_level_set(TAG, ESP_LOG_INFO);
    delay(1000); // Wait for serial port to stabilize
    printSystemInfo();
}

void loop() {
    // Print system information every 10 seconds
    printSystemInfo();
    delay(10000);
}