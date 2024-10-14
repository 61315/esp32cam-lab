#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>
#include <driver/spi_master.h>
#include <esp_log.h>

#define GPIO_MOSI 12
#define GPIO_MISO 13
#define GPIO_SCLK 15
#define GPIO_CS 14

static const char* TAG      = "example:spi_hello3_master";
const char*        ssid     = "TAMU_IoT";
const char*        password = "";

spi_device_handle_t spi_handle;
WebServer           server(80);

char receivedMessage[128] = {0};
char messageToSend[128]   = {0};

void setupSPI() {
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num      = GPIO_MOSI;
    buscfg.miso_io_num      = GPIO_MISO;
    buscfg.sclk_io_num      = GPIO_SCLK;
    buscfg.quadwp_io_num    = -1;
    buscfg.quadhd_io_num    = -1;

    spi_device_interface_config_t devcfg = {};
    devcfg.clock_speed_hz                = 5000000;
    devcfg.mode                          = 0;
    devcfg.spics_io_num                  = GPIO_CS;
    devcfg.queue_size                    = 3;

    esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    assert(ret == ESP_OK);
    ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi_handle);
    assert(ret == ESP_OK);
}

void handleRoot() {
    String html = "<html><body>";
    html += "<h1>SPI Master Chat</h1>";
    html += "<p>Received: " + String(receivedMessage) + "</p>";
    html += "<form method='POST' action='/send'>";
    html += "Message: <input type='text' name='message'>";
    html += "<input type='submit' value='Send'>";
    html += "</form></body></html>";
    server.send(200, "text/html", html);
}

void handleSend() {
    if (server.hasArg("message")) {
        String message = server.arg("message");
        message.toCharArray(messageToSend, sizeof(messageToSend));
        server.sendHeader("Location", "/");
        server.send(303);
    }
}

void spiTransfer() {
    spi_transaction_t t = {};
    t.length            = sizeof(messageToSend) * 8;
    t.tx_buffer         = messageToSend;
    t.rx_buffer         = receivedMessage;

    esp_err_t ret = spi_device_transmit(spi_handle, &t);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "SPI transfer successful");
    } else {
        ESP_LOGE(TAG, "SPI transfer failed");
    }

    memset(messageToSend, 0, sizeof(messageToSend));
}

void setup() {
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector

    Serial.begin(115200);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    setupSPI();

    server.on("/", handleRoot);
    server.on("/send", HTTP_POST, handleSend);
    server.begin();

    ESP_LOGI(TAG, "SPI Master and Web Server initialized");
}

void loop() {
    server.handleClient();
    spiTransfer();
    delay(100);
}