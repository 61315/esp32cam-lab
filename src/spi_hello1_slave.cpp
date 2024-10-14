#include <Arduino.h>
#include <driver/spi_slave.h>
#include <esp_log.h>

// Pins in use
#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS 14

static const char* TAG = "example:spi_hello1_slave";

char                    recvbuf[129] = "";
spi_slave_transaction_t t;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ;
    }

    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num      = GPIO_MOSI;
    buscfg.miso_io_num      = GPIO_MISO;
    buscfg.sclk_io_num      = GPIO_SCLK;
    buscfg.quadwp_io_num    = -1;
    buscfg.quadhd_io_num    = -1;

    spi_slave_interface_config_t slvcfg = {};
    slvcfg.mode                         = 0;
    slvcfg.spics_io_num                 = GPIO_CS;
    slvcfg.queue_size                   = 3;
    slvcfg.flags                        = 0;

    esp_err_t ret = spi_slave_initialize(HSPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI slave");
        return;
    }

    memset(recvbuf, 0, sizeof(recvbuf));
    memset(&t, 0, sizeof(t));

    ESP_LOGI(TAG, "SPI Slave initialized successfully");
}

void loop() {
    t.length    = 128 * 8;
    t.rx_buffer = recvbuf;

    esp_err_t ret = spi_slave_transmit(HSPI_HOST, &t, portMAX_DELAY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI slave transmission failed");
    } else {
        ESP_LOGI(TAG, "Received: %s", recvbuf);
    }
}