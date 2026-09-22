#ifndef MOCK_U8G2_H
#define MOCK_U8G2_H
#include "Arduino.h"

extern const void* u8g2_font_6x10_tf;
extern const void* u8g2_font_5x8_tf;
extern const void* u8g2_font_fub20_tf;
extern const void* u8g2_font_fub30_tf;
extern const void* u8g2_font_helvB14_tf;
extern const void* u8g2_font_6x12_tf;
constexpr int U8G2_R0 = 0;
constexpr int U8X8_PIN_NONE = -1;

class U8G2_SSD1306_128X64_NONAME_F_HW_I2C {
public:
    int outOfBoundsDraws = 0;
    const void* font = nullptr;
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C(int, int) {}
    void begin() {}
    void clearBuffer() {}
    void sendBuffer() {}
    void setFont(const void* value) { font = value; }
    int getStrWidth(const char* value) const {
        int width = font == u8g2_font_5x8_tf ? 5 : (font == u8g2_font_fub20_tf ? 14 : 6);
        return static_cast<int>(std::strlen(value)) * width;
    }
    void drawStr(int x, int y, const char* value) {
        if (x < 0 || y < 0 || y > 64 || x + getStrWidth(value) > 128) outOfBoundsDraws++;
    }
    void drawRBox(int, int, int, int, int) {}
    void drawBox(int, int, int, int) {}
    void drawDisc(int, int, int) {}
    void drawTriangle(int, int, int, int, int, int) {}
    void drawLine(int, int, int, int) {}
    void setDrawColor(int) {}
    void setContrast(uint8_t) {}
};
#endif
