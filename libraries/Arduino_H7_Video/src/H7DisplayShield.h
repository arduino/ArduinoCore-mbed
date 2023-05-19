#pragma once
#include "Arduino.h"

class H7DisplayShield {
    public:
        virtual int init(int edidmode) = 0;
        virtual int getEdidMode(int h, int v);
};

class GigaDisplayShieldClass : public H7DisplayShield {
    public:
        int init(int edidmode);
        int getEdidMode(int h, int v);
};

class USBCVideoClass : public H7DisplayShield {
    public:
        int init(int edidmode);
        int getEdidMode(int h, int v);
};

extern GigaDisplayShieldClass GigaDisplayShield;
extern USBCVideoClass USBCVideo;