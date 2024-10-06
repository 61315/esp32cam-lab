#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

static const char* TAG = "example:heavy_task_parallel";

const int ITERATIONS = 1000000;

TaskHandle_t Task1;
TaskHandle_t Task2;

volatile bool task1Complete = false;
volatile bool task2Complete = false;

void performHeavyTask(void* parameter) {
    const char*  taskName = (const char*)parameter;
    volatile int result   = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        result += i;
    }
    ESP_LOGI(TAG, "%s completed on core %d. Result: %d", taskName, xPortGetCoreID(), result);

    if (strcmp(taskName, "Task1") == 0) {
        task1Complete = true;
    } else if (strcmp(taskName, "Task2") == 0) {
        task2Complete = true;
    }

    vTaskDelete(NULL);
}

void setup() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);
    esp_log_level_set(TAG, ESP_LOG_INFO);
    ESP_LOGI(TAG, "Starting dual core test");
}

void loop() {
    task1Complete = false;
    task2Complete = false;

    unsigned long start = millis();

    xTaskCreatePinnedToCore(performHeavyTask, "Task1", 10000, (void*)"Task1", 1, &Task1, 0);

    xTaskCreatePinnedToCore(performHeavyTask, "Task2", 10000, (void*)"Task2", 1, &Task2, 1);

    // Wait for both tasks to complete
    while (!task1Complete || !task2Complete) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    unsigned long end = millis();

    ESP_LOGI(TAG, "Total execution time: %lu ms", end - start);
    ESP_LOGI(TAG, "--------------------");

    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 second delay between iterations
}