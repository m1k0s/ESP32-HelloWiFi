#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>

#if DISPLAY_U8G2
#include <U8g2lib.h>
#elif DISPLAY_TFT_ESPI
#include <TFT_eSPI.h>
#elif DISPLAY_WROVER_KIT_LCD
#include <WROVER_KIT_LCD.h>
#endif

class Display
{
private:
#if DISPLAY_U8G2
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C m_OLED;
#elif DISPLAY_TFT_ESPI
    TFT_eSPI m_TFT;
    TFT_eSprite m_Buffer;
#elif DISPLAY_WROVER_KIT_LCD
    WROVER_KIT_LCD m_TFT;
#endif

public:
    const static uint16_t BLACK = 0x0000;
    const static uint16_t WHITE = 0xffff;

public:
    Display()
#if DISPLAY_U8G2
        : m_OLED(U8G2_R0, OLED_RESET, OLED_CLOCK, OLED_DATA)
#elif DISPLAY_TFT_ESPI
        : m_TFT(), m_Buffer(&m_TFT)
#elif DISPLAY_WROVER_KIT_LCD
        : m_TFT()
#endif
    {
    }

    void Init()
    {
#if DISPLAY_U8G2
        m_OLED.begin();
        m_OLED.clear();
        m_OLED.setFont(u8g2_font_profont10_tf);
        m_OLED.setFontPosTop();
#elif DISPLAY_TFT_ESPI
        m_TFT.begin();
        m_TFT.setRotation(3);
        m_Buffer.setColorDepth(16);
        m_Buffer.createSprite(m_TFT.width(), m_TFT.height());
        m_Buffer.setTextColor(WHITE, BLACK);
        m_Buffer.setTextFont(2);
#elif DISPLAY_WROVER_KIT_LCD
        m_TFT.begin();
        m_TFT.setRotation(3);
        m_TFT.setTextColor(WHITE);
        m_TFT.setTextSize(1);
#endif
    }

    int16_t Width()
    {
#if DISPLAY_U8G2
        return m_OLED.getDisplayWidth();
#elif DISPLAY_TFT_ESPI
        return m_TFT.width();
#elif DISPLAY_WROVER_KIT_LCD
        return m_TFT.width();
#else
        return 0;
#endif
    }

    int16_t Height()
    {
#if DISPLAY_U8G2
        return m_OLED.getDisplayHeight();
#elif DISPLAY_TFT_ESPI
        return m_TFT.height();
#elif DISPLAY_WROVER_KIT_LCD
        return m_TFT.height();
#else
        return 0;
#endif
    }

    void Clear(uint16_t color = 0)
    {
#if DISPLAY_U8G2
        m_OLED.clearBuffer();
#elif DISPLAY_TFT_ESPI
        m_Buffer.fillSprite(color);
#elif DISPLAY_WROVER_KIT_LCD
        m_TFT.fillScreen(color);
#endif
    }

    void SendBuffer()
    {
#if DISPLAY_U8G2
        m_OLED.sendBuffer();
#elif DISPLAY_TFT_ESPI
        m_Buffer.pushSprite(0, 0);
#elif DISPLAY_WROVER_KIT_LCD
#endif
    }

    int16_t FontMaxCharWidth()
    {
#if DISPLAY_U8G2
        return m_OLED.getMaxCharWidth();
#elif DISPLAY_TFT_ESPI
        return m_Buffer.textWidth("M");
#elif DISPLAY_WROVER_KIT_LCD
        int16_t x1;
        int16_t y1;
        uint16_t w;
        uint16_t h;
        m_TFT.getTextBounds("M", 0, 0, &x1, &y1, &w, &h);
        return w;
#else
        return 0;
#endif
    }

    int16_t FontHeight()
    {
#if DISPLAY_U8G2
        return m_OLED.getFontAscent() - m_OLED.getFontDescent();
#elif DISPLAY_TFT_ESPI
        return m_Buffer.fontHeight();
#elif DISPLAY_WROVER_KIT_LCD
        int16_t x1;
        int16_t y1;
        uint16_t w;
        uint16_t h;
        m_TFT.getTextBounds("M", 0, 0, &x1, &y1, &w, &h);
        return h;
#else
        return 0;
#endif
    }

    void DrawString(int16_t x, int16_t y, const char *s, size_t len)
    {
#if DISPLAY_U8G2
        m_OLED.setCursor(x, y);
        m_OLED.write((uint8_t*)s, len);
#elif DISPLAY_TFT_ESPI
        m_Buffer.drawString(s, x, y);
#elif DISPLAY_WROVER_KIT_LCD
        m_TFT.setCursor(x, y);
        m_TFT.write(s, len);
#endif
    }

    void DrawPixel(int16_t x, int16_t y, uint16_t color = WHITE)
    {
#if DISPLAY_U8G2
        m_OLED.drawPixel(x, y);
#elif DISPLAY_TFT_ESPI
        m_Buffer.drawPixel(x, y, color);
#elif DISPLAY_WROVER_KIT_LCD
        m_TFT.drawPixel(x, y, color);
#endif
    }

    void DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color = WHITE)
    {
#if DISPLAY_U8G2
        m_OLED.drawLine(x0, y0, x1, y1);
#elif DISPLAY_TFT_ESPI
        m_Buffer.drawLine(x0, y0, x1, y1, color);
#elif DISPLAY_WROVER_KIT_LCD
        m_TFT.drawLine(x0, y0, x1, y1, color);
#endif
    }

    void DrawRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color = WHITE)
    {
#if DISPLAY_U8G2
        m_OLED.drawFrame(x0, y0, x1, y1);
#elif DISPLAY_TFT_ESPI
        m_Buffer.drawRect(x0, y0, x1, y1, color);
#elif DISPLAY_WROVER_KIT_LCD
        m_TFT.drawRect(x0, y0, x1, y1, color);
#endif
    }

    void FillRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color = WHITE)
    {
#if DISPLAY_U8G2
        m_OLED.drawBox(x0, y0, x1, y1);
#elif DISPLAY_TFT_ESPI
        m_Buffer.fillRect(x0, y0, x1, y1, color);
#elif DISPLAY_WROVER_KIT_LCD
        m_TFT.fillRect(x0, y0, x1, y1, color);
#endif
    }

    void PrintF(int16_t x, int16_t y, const char *format, ...)
    {
        char buffer[256];
        va_list args;
        va_start(args, format);
        int len = vsnprintf(buffer, sizeof(buffer), format, args);
        va_end(args);
        DrawString(x, y, buffer, len);
    }
};
