#include "drivers/InterruptIn.h"

enum {
    CAMERA_R160x120 = 0x00,   /* QQVGA Resolution   */
    CAMERA_R320x240 = 0x01,   /* QVGA Resolution    */
    CAMERA_R320x320 = 0x02,   /* 320x320 Resolution */
    CAMERA_RMAX
};

typedef void (*md_callback_t)();

class CameraClass {
    private:
        uint32_t resolution;
        bool     initialized;
        mbed::InterruptIn md_irq;
        void HIMAXIrqHandler();
    public:
        CameraClass(): initialized(false), md_irq(PC_15){}
        int begin(uint32_t resolution = CAMERA_R320x240, uint32_t framerate = 30);
        int framerate(uint32_t framerate);
        int grab(uint8_t *buffer, uint32_t timeout=5000);
        int standby(bool enable);
        int motionDetection(bool enable, md_callback_t callback=NULL);
        int motionDetectionWindow(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
        int motionDetectionThreshold(uint32_t threshold);
        int motionDetected();
        int testPattern(bool walking);
};
