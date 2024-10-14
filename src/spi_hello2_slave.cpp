#include <Arduino.h>
#include <driver/spi_slave.h>
#include <esp_log.h>

// Pins in use
#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS 14

static const char* TAG = "example:spi_hello2_slave";

int                     counter      = 0; // Counter
char                    sendbuf[128] = {0};
spi_slave_transaction_t t;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ;
    }

    // Configuration for the SPI bus
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num      = GPIO_MOSI;
    buscfg.miso_io_num      = GPIO_MISO;
    buscfg.sclk_io_num      = GPIO_SCLK;
    buscfg.quadwp_io_num    = -1;
    buscfg.quadhd_io_num    = -1;

    // Configuration for the SPI slave interface
    spi_slave_interface_config_t slvcfg = {};
    slvcfg.mode                         = 0;
    slvcfg.spics_io_num                 = GPIO_CS;
    slvcfg.queue_size                   = 3;
    slvcfg.flags                        = 0;

    // Initialize SPI slave interface
    esp_err_t ret = spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI slave");
        return;
    }

    // Initialize transaction
    memset(&t, 0, sizeof(t));

    ESP_LOGI(TAG, "SPI Slave initialized successfully");
}

void loop() {
    snprintf(sendbuf, sizeof(sendbuf), "Sent by slave - %d", counter);
    t.length    = sizeof(sendbuf) * 8;
    t.tx_buffer = sendbuf;

    esp_err_t ret = spi_slave_transmit(HSPI_HOST, &t, portMAX_DELAY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI slave transmission failed");
    } else {
        ESP_LOGI(TAG, "Transmitted: %s", sendbuf);
    }

    counter++;
    delay(1000);
}