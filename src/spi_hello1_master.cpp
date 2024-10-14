#include <Arduino.h>
#include <driver/spi_master.h>
#include <esp_log.h>

#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS 14

static const char* TAG = "example:spi_hello1_master";

spi_device_handle_t spi_handle;
char                sendbuf[128] = {0};
spi_transaction_t   t;

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

    // Configuration for the SPI device on the other side of the bus
    spi_device_interface_config_t devcfg = {};
    devcfg.command_bits                  = 0;
    devcfg.address_bits                  = 0;
    devcfg.dummy_bits                    = 0;
    devcfg.clock_speed_hz                = 5000000;
    devcfg.duty_cycle_pos                = 128; // 50% duty cycle
    devcfg.mode                          = 0;
    devcfg.spics_io_num                  = GPIO_CS;
    devcfg.cs_ena_posttrans              = 3; // Keep the CS low 3 cycles after transaction
    devcfg.queue_size                    = 3;

    esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus");
        return;
    }

    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device");
        return;
    }

    // Initialize transaction
    memset(&t, 0, sizeof(t));

    ESP_LOGI(TAG, "SPI Master initialized successfully");
}

void loop() {
    snprintf(sendbuf, sizeof(sendbuf), "yo");
    t.length    = sizeof(sendbuf) * 8;
    t.tx_buffer = sendbuf;

    esp_err_t ret = spi_device_transmit(spi_handle, &t);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SPI transmission failed");
    } else {
        ESP_LOGI(TAG, "Transmitted: %s", sendbuf);
    }

    delay(1000);
}