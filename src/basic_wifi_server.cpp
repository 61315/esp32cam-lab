#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_system.h>
#include "soc/soc.h"           // for brownout disable
#include "soc/rtc_cntl_reg.h"  // for brownout disable

const char* ssid     = "asdqwe";
const char* password = "22222222";

WebServer server(80);
static const char* TAG = "example:basicwifi_server";

// function prototypes
void setup_wifi_ap();
void setup_routes();
void handle_root();

void setup() {
    // disable brownout detector
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(1000);  // wait for serial to initialize

    ESP_LOGI(TAG, "Starting WiFi Access Point...");
    setup_wifi_ap();
    setup_routes();
}

void loop() {
    server.handleClient();
    delay(10);  // small delay to prevent watchdog issues
}

void setup_wifi_ap() {
    WiFi.softAP(ssid, password);
    ESP_LOGI(TAG, "Access Point Started");
    ESP_LOGI(TAG, "IP Address: %s", WiFi.softAPIP().toString().c_str());
}

void setup_routes() {
    server.on("/", HTTP_GET, handle_root);
    server.begin();
    ESP_LOGI(TAG, "HTTP server started");
}

void handle_root() {
    unsigned long uptime = millis() / 1000;  // convert to seconds
    unsigned long hours = uptime / 3600;
    unsigned long minutes = (uptime % 3600) / 60;
    unsigned long seconds = uptime % 60;

    char html[512];
    snprintf(html, sizeof(html),
        "<html><head>"
        "<meta http-equiv='refresh' content='1'/>"  // auto refresh every 1 second
        "<title>ESP32-CAM Server</title>"
        "</head><body>"
        "<h1>ESP32-CAM Simple Server</h1>"
        "<p>System Uptime: %02lu:%02lu:%02lu</p>"
        "</body></html>",
        hours, minutes, seconds
    );

    server.send(200, "text/html", html);
}