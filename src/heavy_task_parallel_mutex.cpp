#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <Arduino.h>

static const char* TAG = "example:heavy_task_parallel_mutex";

const int ITERATIONS = 1000000;

TaskHandle_t Task1;
TaskHandle_t Task2;

volatile bool task1Complete = false;
volatile bool task2Complete = false;

SemaphoreHandle_t logMutex;

void synchronizedLog(const char* format, ...) {
    xSemaphoreTake(logMutex, portMAX_DELAY);
    va_list args;
    va_start(args, format);
    esp_log_writev(ESP_LOG_INFO, TAG, format, args);
    va_end(args);
    printf("\n");
    xSemaphoreGive(logMutex);
}

void performHeavyTask(void* parameter) {
    const char*  taskName = (const char*)parameter;
    volatile int result   = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        result += i;
    }
    synchronizedLog("%s completed on core %d. Result: %d", taskName, xPortGetCoreID(), result);

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

    logMutex = xSemaphoreCreateMutex();

    synchronizedLog("Starting dual core test");
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

    synchronizedLog("Total execution time: %lu ms", end - start);
    synchronizedLog("--------------------");

    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1 second delay between iterations
}