#include "Adafruit_GFX.h"
#include "driver/spi_slave.h"
#include "soc/spi_reg.h"
#include <Arduino.h>
#include <SPI.h>
#include <esp_camera.h>

// bitmap dimensions (qqvga)
#define BITMAP_WIDTH 160
#define BITMAP_HEIGHT 120

// spi pins for slave
#define SPI_MOSI 13
#define SPI_MISO 12
#define SPI_SCLK 14
#define SPI_CS 15

// DMA buffers
static uint8_t* bitmap = nullptr;
static uint8_t* jpeg_buffer = nullptr;
static uint32_t* jpeg_size_dma = nullptr;
static size_t jpeg_len = 0;

// frame counter
static uint32_t frame_count = 0;

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
static gray_bitmap* gfx = nullptr;

// moving text position
static int text_x = 0;
static int text_y = 30;
static int direction_x = 1;

// generate new frame
void generate_frame() {
    uint32_t now = millis();

    // clear bitmap
    gfx->fillScreen(0);

    // draw animated border
    uint8_t border_color = (now / 500) % 255;
    gfx->drawRect(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, border_color);

    // draw moving text
    gfx->setTextColor(255);
    gfx->setTextSize(2);
    gfx->setCursor(text_x, text_y);
    gfx->print("slave");

    // draw diagonal line
    gfx->drawLine(0, 0, BITMAP_WIDTH, BITMAP_HEIGHT, 128);

    // draw moving circle
    int circle_x = BITMAP_WIDTH / 2 + sin(now / 1000.0) * 30;
    int circle_y = BITMAP_HEIGHT / 2 + cos(now / 1000.0) * 20;
    gfx->fillCircle(circle_x, circle_y, 10, 200);

    // update text position
    text_x += direction_x * 2;
    if (text_x >= BITMAP_WIDTH - 50 || text_x <= 0) {
        direction_x *= -1;
    }

    // add frame info
    gfx->setTextSize(1);
    gfx->setCursor(3, BITMAP_HEIGHT - 24);
    gfx->printf("frame: %d", frame_count++);
    gfx->setCursor(3, BITMAP_HEIGHT - 12);
    gfx->printf("time: %lu ms", now);
}

void setup() {
    Serial.begin(115200);
    delay(1000); // wait for serial to stabilize

    // IMPORTANT: allocate 32-bit aligned DMA-capable memory ffs
    // round up sizes to multiples of 4 bytes
    size_t bitmap_size = ((BITMAP_WIDTH * BITMAP_HEIGHT + 3) & ~3);
    size_t jpeg_buffer_size = ((BITMAP_WIDTH * BITMAP_HEIGHT * 2 + 3) & ~3);
    
    bitmap = (uint8_t*)heap_caps_aligned_alloc(4, bitmap_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    jpeg_buffer = (uint8_t*)heap_caps_aligned_alloc(4, jpeg_buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);
    jpeg_size_dma = (uint32_t*)heap_caps_aligned_alloc(4, sizeof(uint32_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    if (!bitmap || !jpeg_buffer || !jpeg_size_dma) {
        Serial.println("Failed to allocate DMA memory");
        return;
    }

    // initialize GFX
    gfx = new gray_bitmap(bitmap, BITMAP_WIDTH, BITMAP_HEIGHT);

    // configure SPI slave
    spi_bus_config_t buscfg = {
        .mosi_io_num = SPI_MOSI,
        .miso_io_num = SPI_MISO,
        .sclk_io_num = SPI_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = BITMAP_WIDTH * BITMAP_HEIGHT * 2,
        .flags = 0,
        .intr_flags = 0
    };

    spi_slave_interface_config_t slvcfg = {};
    slvcfg.mode                         = 0;
    slvcfg.spics_io_num                 = SPI_CS;
    slvcfg.queue_size                   = 3;
    slvcfg.flags                        = 0;

    // initialize SPI slave
    esp_err_t ret = spi_slave_initialize(VSPI_HOST, &buscfg, &slvcfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        Serial.println("Failed to initialize SPI slave");
        return;
    }

    Serial.println("SPI slave initialized successfully");
}

void loop() {
    static spi_slave_transaction_t t;
    memset(&t, 0, sizeof(t)); // clear transaction struct

    // generate new frame
    generate_frame();

    // convert to JPEG - using temporary buffer first
    uint8_t* temp_jpeg = NULL;
    bool jpeg_converted = fmt2jpg(bitmap, BITMAP_WIDTH * BITMAP_HEIGHT, 
                                BITMAP_WIDTH, BITMAP_HEIGHT,
                                PIXFORMAT_GRAYSCALE, 20, 
                                &temp_jpeg, &jpeg_len);

    if (!jpeg_converted || !temp_jpeg) {
        Serial.println("JPEG conversion failed");
        delay(100);
        return;
    }

    // copy to DMA buffer
    if (jpeg_len > BITMAP_WIDTH * BITMAP_HEIGHT * 2) {
        Serial.println("JPEG too large for buffer");
        free(temp_jpeg);
        delay(100);
        return;
    }
    
    memcpy(jpeg_buffer, temp_jpeg, jpeg_len);
    free(temp_jpeg);  // free temporary buffer

    // store JPEG size in DMA buffer
    *jpeg_size_dma = jpeg_len;

    // first transaction: send JPEG size
    t.length = 32; // 4 bytes * 8 bits
    t.tx_buffer = jpeg_size_dma;
    
    esp_err_t ret = spi_slave_transmit(VSPI_HOST, &t, portMAX_DELAY);
    if (ret != ESP_OK) {
        Serial.println("Failed to send JPEG size");
        delay(100);
        return;
    }

    // second transaction: send JPEG data
    t.length = jpeg_len * 8;
    t.tx_buffer = jpeg_buffer;

    ret = spi_slave_transmit(VSPI_HOST, &t, portMAX_DELAY);
    if (ret != ESP_OK) {
        Serial.println("Failed to send JPEG data");
        delay(100);
        return;
    }

    Serial.printf("Frame %d sent, size: %d bytes\n", frame_count, jpeg_len);
    delay(50); // ~20fps
}