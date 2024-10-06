#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

static const char* TAG = "example:dual_core_test1";

// Task handles
TaskHandle_t Task1;
TaskHandle_t Task2;

// Task for Core 0
void codeForTask1(void* parameter) {
    for (;;) {
        ESP_LOGI(TAG, "Task 1 running on core %d", xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

// Task for Core 1
void codeForTask2(void* parameter) {
    for (;;) {
        ESP_LOGI(TAG, "Task 2 running on core %d", xPortGetCoreID());
        vTaskDelay(pdMS_TO_TICKS(1000)); // Delay for 1 second
    }
}

void setup() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_log_level_set(TAG, ESP_LOG_INFO);

    ESP_LOGI(TAG, "Starting dual core test");

    // Create Task 1 and pin it to Core 0
    xTaskCreatePinnedToCore(codeForTask1, // Function to implement the task
                            "Task1",      // Name of the task
                            10000,        // Stack size in words
                            NULL,         // Task input parameter
                            1,            // Priority of the task
                            &Task1,       // Task handle
                            0             // Core where the task should run
    );

    // Create Task 2 and pin it to Core 1
    xTaskCreatePinnedToCore(codeForTask2, // Function to implement the task
                            "Task2",      // Name of the task
                            10000,        // Stack size in words
                            NULL,         // Task input parameter
                            1,            // Priority of the task
                            &Task2,       // Task handle
                            1             // Core where the task should run
    );
}

void loop() {
    // The loop function is empty as we're using FreeRTOS tasks
    vTaskDelay(pdMS_TO_TICKS(1000));
}