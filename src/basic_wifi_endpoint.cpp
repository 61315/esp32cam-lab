#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <esp_system.h>
#include "soc/soc.h"           // for brownout disable
#include "soc/rtc_cntl_reg.h"  // for brownout disable

// WiFi credentials
const char* ssid     = "TAMU_IoT";
const char* password = "22222222";

WebServer server(80);
static const char* TAG = "example:basic_wifi_endpoint";

// function prototypes
void connect_wifi();
void setup_routes();
void handle_root();

// WiFi connection retry parameters
const int MAX_RETRY = 10;
const int RETRY_DELAY = 500;  // milliseconds

void setup() {
    // disable brownout detector
    WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

    Serial.begin(115200);
    delay(1000);  // wait for serial to initialize

    ESP_LOGI(TAG, "Connecting to WiFi...");
    connect_wifi();
    setup_routes();
}

void loop() {
    // check if WiFi is still connected
    if(WiFi.status() != WL_CONNECTED) {
        ESP_LOGW(TAG, "WiFi connection lost. Reconnecting...");
        connect_wifi();
    }

    server.handleClient();
    delay(10);  // small delay to prevent watchdog issues
}

void connect_wifi() {
    WiFi.mode(WIFI_STA);  // set as a station (client)
    WiFi.begin(ssid, password);

    int retry_count = 0;
    while (WiFi.status() != WL_CONNECTED && retry_count < MAX_RETRY) {
        delay(RETRY_DELAY);
        ESP_LOGI(TAG, "Connecting to WiFi... (Attempt %d/%d)", retry_count + 1, MAX_RETRY);
        retry_count++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        ESP_LOGI(TAG, "Connected to WiFi successfully!");
        ESP_LOGI(TAG, "Endpoint is Ready! Go to: http://%s", WiFi.localIP().toString().c_str());
        ESP_LOGI(TAG, "Signal Strength (RSSI): %d dBm", WiFi.RSSI());
    } else {
        ESP_LOGE(TAG, "Failed to connect to WiFi after %d attempts", MAX_RETRY);
        ESP_LOGE(TAG, "Restarting...");
        delay(1000);
        ESP.restart();
    }
}

void setup_routes() {
    server.on("/", HTTP_GET, handle_root);
    server.begin();
    ESP_LOGI(TAG, "HTTP server started");
}

void handle_root() {
    char html[1024];
    snprintf(html, sizeof(html),
        "<html><head>"
        "<meta http-equiv='refresh' content='5'/>"  // refresh every 5 seconds
        "<title>ESP32-CAM WiFi Status</title>"
        "<style>"
        "body { font-family: Arial, sans-serif; margin: 20px; }"
        "table { border-collapse: collapse; }"
        "td, th { border: 1px solid #ddd; padding: 8px; }"
        "th { background-color: #f2f2f2; }"
        "</style>"
        "</head><body>"
        "<h1>ESP32-CAM WiFi Status</h1>"
        "<table>"
        "<tr><th>Parameter</th><th>Value</th></tr>"
        "<tr><td>WiFi SSID</td><td>%s</td></tr>"
        "<tr><td>IP Address</td><td>%s</td></tr>"
        "<tr><td>Signal Strength</td><td>%d dBm</td></tr>"
        "<tr><td>MAC Address</td><td>%s</td></tr>"
        "<tr><td>Uptime</td><td>%lu seconds</td></tr>"
        "</table>"
        "<p>Page auto-refreshes every 5 seconds</p>"
        "</body></html>",
        WiFi.SSID().c_str(),
        WiFi.localIP().toString().c_str(),
        WiFi.RSSI(),
        WiFi.macAddress().c_str(),
        millis() / 1000
    );

    server.send(200, "text/html", html);
}