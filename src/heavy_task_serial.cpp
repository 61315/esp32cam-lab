#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

static const char* TAG = "example:heavy_task_serial";

const int ITERATIONS = 1000000;

void performHeavyTask(const char* taskName) {
    volatile int result = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        result += i;
    }
    ESP_LOGI(TAG, "%s completed. Result: %d", taskName, result);
}

void setup() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "Starting single core test");
}

void loop() {
    unsigned long start = millis();

    // Perform Task 1
    performHeavyTask("Task1");

    // Perform Task 2
    performHeavyTask("Task2");

    unsigned long end = millis();

    ESP_LOGI(TAG, "Total execution time: %lu ms", end - start);
    ESP_LOGI(TAG, "--------------------");

    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 second delay between iterations
}