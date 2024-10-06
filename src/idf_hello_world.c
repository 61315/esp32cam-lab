#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "example:idf_hello_world";

void app_main(void) {
    ESP_LOGI(TAG, "Hello World!");

    while (1) {
        ESP_LOGI(TAG, "Doing something...");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}