#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

static const char* TAG = "example:math_dotprod_naive";

#define N_SAMPLES 1024

__attribute__((aligned(16))) float input1[N_SAMPLES];
__attribute__((aligned(16))) float input2[N_SAMPLES];

float dotprod_f32(const float* input1, const float* input2, int len) {
    float result = 0;
    for (int i = 0; i < len; i++) {
        result += input1[i] * input2[i];
    }
    return result;
}

void generate_random_input(float* input, int len) {
    for (int i = 0; i < len; i++) {
        input[i] = (float)esp_random() / UINT32_MAX;
    }
}

void heavy_task(void* pvParameters) {
    while (1) {
        // Generate random inputs
        generate_random_input(input1, N_SAMPLES);
        generate_random_input(input2, N_SAMPLES);

        float        result       = 0;
        unsigned int start_cycles = esp_cpu_get_ccount();
        int64_t      start_time   = esp_timer_get_time();

        // Perform dot product
        result = dotprod_f32(input1, input2, N_SAMPLES);

        unsigned int end_cycles = esp_cpu_get_ccount();
        int64_t      end_time   = esp_timer_get_time();

        ESP_LOGI(TAG, "Dot product result = %f", result);
        ESP_LOGI(TAG, "Operation took %u cycles", end_cycles - start_cycles);
        ESP_LOGI(TAG, "Operation took %lld microseconds", end_time - start_time);

        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void setup() {
    vTaskDelay(2000 / portTICK_PERIOD_MS);

    xTaskCreatePinnedToCore(heavy_task, "heavy_task", 8192, NULL, 5, NULL, 0);
}

void loop() {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}