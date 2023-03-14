#ifndef PORTENTA_VIDEO_H
#define PORTENTA_VIDEO_H

#include "Arduino.h"
#include "anx7625.h"
#include "dsi.h"

#define PV_COLOR_BLACK          (0x000000)
#define PV_COLOR_WHITE          (0xFFFFFF)
#define PV_COLOR_RED            (0xFF0000)
#define PV_COLOR_LIME           (0x00FF00)
#define PV_COLOR_BLUE           (0x0000FF)
#define PV_COLOR_YELLOW         (0xFFFF00)
#define PV_COLOR_CYAN           (0x00FFFF)
#define PV_COLOR_MAGENTA        (0xFF00FF)
#define PV_COLOR_SILVER         (0xC0C0C0)
#define PV_COLOR_GRAY           (0x808080)
#define PV_COLOR_MAROON         (0x800000)
#define PV_COLOR_OLIVE          (0x808000)
#define PV_COLOR_GREEN          (0x008000)
#define PV_COLOR_PURPLE         (0x800080)
#define PV_COLOR_TEAL           (0x008080)
#define PV_COLOR_NAVY           (0x000080)

namespace arduino {

    class Portenta_Video {
    public:
        Portenta_Video();
        int begin();
        void fillScreen(uint32_t color);
        void drawImage(void *imgBuffer, uint32_t imgWidth, uint32_t imgHeight, uint32_t posWidht, uint32_t posHeight);
        void drawPixel(uint32_t posWidht, uint32_t posHeight, uint32_t color);
        void drawLine(int x0, int y0, int x1, int y1, uint32_t color);
        void drawRectangle(uint32_t rectWidth, uint32_t rectHeight, uint32_t posWidht, uint32_t posHeight, uint32_t color);
        void drawFilledCircle(uint32_t centerX, uint32_t centerY, uint32_t radius, uint32_t color);
        void drawCircle(uint32_t centerX, uint32_t centerY, uint32_t radius, uint32_t color);
        void clear();
        void updateDisplay();
        uint32_t getWidthSize();
        uint32_t getHeightSize();
    private:
        uint32_t _currFrameBufferAddr;
        uint32_t _displayWidth;
        uint32_t _displayHeight;
    };  

}
#endif /* PORTENTA_VIDEO_H */