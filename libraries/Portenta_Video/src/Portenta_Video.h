#ifndef PORTENTA_VIDEO_H
#define PORTENTA_VIDEO_H

#include "Arduino.h"
#include "anx7625.h"
#include "dsi.h"

#define PV_COLOR_BLACK      (0x000000)
#define PV_COLOR_WHITE      (0xFFFFFF)
#define PV_COLOR_RED        (0xFF0000)
#define PV_COLOR_GREEN      (0x00FF00)
#define PV_COLOR_BLUE       (0x0000FF)
#define PV_COLOR_YELLOW     (0xFFFF00)
#define PV_COLOR_MAGENTA    (0xFF00FF)
#define PV_COLOR_CYAN       (0x00FFFF)
#define PV_COLOR_ORANGE     (0xFFA500)
#define PV_COLOR_PINK       (0xFFC0CB)
#define PV_COLOR_PURPLE     (0x800080)
#define PV_COLOR_GRAY       (0x808080)

enum DisplayShieldModel {
    NONE_SHIELD = 0,
    GIGA_DISPLAY_SHIELD = 1
};

namespace arduino {

    class Portenta_Video {
    public:
        Portenta_Video(DisplayShieldModel shield = NONE_SHIELD);

        int         begin();
        void        fillScreen(uint32_t color);
        void        clear();
        void        update();
        uint32_t    getWidth();
        uint32_t    getHeight();

        void drawImage(uint32_t x, uint32_t y, void *img, uint32_t width, uint32_t height);
        void drawPixel(uint32_t x, uint32_t y, uint32_t color);
        void drawLine(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color);
        void drawRectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
        void drawFilledRectangle(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint32_t color);
        void drawCircle(uint32_t centerX, uint32_t centerY, uint32_t radius, uint32_t color);
        void drawFilledCircle(uint32_t centerX, uint32_t centerY, uint32_t radius, uint32_t color);
        void drawChar(uint32_t x, uint32_t y, unsigned char c, uint32_t color, uint32_t bg, uint8_t size);      //@TODO: Manage orientation?
        void drawText(uint32_t x, uint32_t y, const char *text, uint32_t color, uint32_t bg, uint8_t size);     //@TODO: Manage orientation?
    private:
        DisplayShieldModel  _shield;
        uint32_t            _currFrameBufferAddr;
        uint32_t            _displayWidth;
        uint32_t            _displayHeight;
    };  

}
#endif /* PORTENTA_VIDEO_H */