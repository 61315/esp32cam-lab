#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>
#include <math.h>

static const char* TAG = "example:math_fft_naive";

#define N_SAMPLES 256
float x[N_SAMPLES];
float y[N_SAMPLES * 2]; // Complex output

void fft(float* x, float* y, int n) {
    for (int k = 0; k < n; k++) {
        float real = 0;
        float imag = 0;
        for (int t = 0; t < n; t++) {
            float angle = 2 * M_PI * t * k / n;
            real += x[t] * cos(angle);
            imag -= x[t] * sin(angle);
        }
        y[k * 2]     = real; // Normalize by dividing by n
        y[k * 2 + 1] = imag; // Normalize by dividing by n
    }
}

void heavy_task(void* pvParameters) {
    // Generate complex input signal
    for (int i = 0; i < N_SAMPLES; i++) {
        // Combination of different frequency components
        x[i] = 0.5 * sin(2 * M_PI * 10 * i / N_SAMPLES)   // 10 Hz component
               + 0.3 * sin(2 * M_PI * 20 * i / N_SAMPLES) // 20 Hz component
               + 0.2 * sin(2 * M_PI * 30 * i / N_SAMPLES) // 30 Hz component
               + 0.1 * ((float)rand() / RAND_MAX - 0.5);  // Random noise
    }

    while (1) {
        int64_t start = esp_timer_get_time();

        // Perform FFT
        fft(x, y, N_SAMPLES);

        int64_t end = esp_timer_get_time();

        // Calculate power spectrum for the first few points
        for (int i = 0; i < 16; i++) {
            float power = y[i * 2] * y[i * 2] + y[i * 2 + 1] * y[i * 2 + 1];
            ESP_LOGI(TAG, "Power at frequency %d: %.4f", i, power);
        }

        ESP_LOGI(TAG, "Time taken: %lld milliseconds", (end - start) / 1000);

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