#ifndef PORTENTA_VIDEO_H
#define PORTENTA_VIDEO_H

#include "Arduino.h"
#include "anx7625.h"
#include "dsi.h"

namespace arduino {

    class Portenta_Video {
    public:
        Portenta_Video();
        int begin();
        void fillScreen(uint32_t color);
        void drawImage(void *imgBuffer, uint32_t imgWidth, uint32_t imgHeight, uint32_t posWidht, uint32_t posHeight);
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