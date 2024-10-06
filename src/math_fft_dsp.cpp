#include "esp_dsp.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <Arduino.h>

static const char* TAG = "example:math_fft_dsp";

#define N_SAMPLES 256
__attribute__((aligned(16))) float x[N_SAMPLES];
__attribute__((aligned(16))) float y[N_SAMPLES * 2];

void heavy_task(void* pvParameters) {
    esp_err_t ret = dsps_fft2r_init_fc32(NULL, CONFIG_DSP_MAX_FFT_SIZE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Not possible to initialize FFT. Error = %i", ret);
        return;
    }

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

        // Convert real input signal to complex
        for (int i = 0; i < N_SAMPLES; i++) {
            y[i * 2]     = x[i];
            y[i * 2 + 1] = 0;
        }

        // Perform FFT
        dsps_fft2r_fc32(y, N_SAMPLES);
        dsps_bit_rev_fc32(y, N_SAMPLES);
        dsps_cplx2reC_fc32(y, N_SAMPLES);

        int64_t end = esp_timer_get_time();

        // Calculate power spectrum for the first few points
        for (int i = 0; i < 16; i++) {
            float power = y[i * 2] * y[i * 2] + y[i * 2 + 1] * y[i * 2 + 1];
            ESP_LOGI(TAG, "Power at frequency %d: %.4f", i, power);
        }

        ESP_LOGI(TAG, "Time taken: %.4f milliseconds", (end - start) / 1000.0f);

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