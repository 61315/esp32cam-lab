#include <Arduino.h>
#include "Adafruit_GFX.h"

// choose between 160x120 (qqvga) and 320x240 (qvga)
#define BITMAP_WIDTH 160
#define BITMAP_HEIGHT 120

// bitmap buffer
uint8_t bitmap[BITMAP_WIDTH * BITMAP_HEIGHT];

// gfx class for grayscale bitmap
class gray_bitmap : public Adafruit_GFX {
public:
    gray_bitmap(uint8_t* buffer, int16_t w, int16_t h) 
        : Adafruit_GFX(w, h), buffer_(buffer) {}

    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (x < 0 || x >= _width || y < 0 || y >= _height) return;
        buffer_[y * _width + x] = color & 0xFF;
    }

    uint8_t* get_buffer() { return buffer_; }

private:
    uint8_t* buffer_;
};

// global gfx object
gray_bitmap* gfx = nullptr;

// moving text position
int text_x = 0;
int text_y = 30;
int direction_x = 1;

void setup() {
    Serial.begin(115200);
    
    // initialize gfx object
    gfx = new gray_bitmap(bitmap, BITMAP_WIDTH, BITMAP_HEIGHT);
    
    Serial.println("bitmap test started!");
}

void loop() {
    // clear bitmap (black)
    gfx->fillScreen(0);
    
    // draw animated border
    uint8_t border_color = (millis() / 500) % 255; // color changes with time
    gfx->drawRect(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, border_color);
    
    // draw moving text
    gfx->setTextColor(255); // white
    gfx->setTextSize(2);
    gfx->setCursor(text_x, text_y);
    gfx->print("esp32");

    // draw diagonal line
    gfx->drawLine(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, 128); // gray
    
    // draw moving circle
    int circle_x = BITMAP_WIDTH/2 + sin(millis()/1000.0) * 30;
    int circle_y = BITMAP_HEIGHT/2 + cos(millis()/1000.0) * 20;
    gfx->fillCircle(circle_x, circle_y, 10, 200);

    // update text position
    text_x += direction_x * 2;
    if (text_x >= BITMAP_WIDTH - 50 || text_x <= 0) {
        direction_x *= -1;
    }

    // print buffer content via serial (for debugging)
    Serial.printf("frame generated at %lu ms\n", millis());
    // print first few bytes
    Serial.print("first few bytes: ");
    for(int i = 0; i < 10; i++) {
        Serial.printf("%d ", bitmap[i]);
    }
    Serial.println();

    // wait for a second
    delay(1000);
}