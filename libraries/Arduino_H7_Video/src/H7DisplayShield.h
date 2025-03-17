#pragma once
#include "Arduino.h"

class H7DisplayShield {
    public:
        virtual int init(int edidmode) = 0;
        virtual int getEdidMode(int h, int v);
        virtual int getStatus();
};

class GigaDisplayShieldClass : public H7DisplayShield {
    public:
        int init(int edidmode);
        int getEdidMode(int h, int v);
        int getStatus();
};

class USBCVideoClass : public H7DisplayShield {
    public:
        int init(int edidmode);
        int getEdidMode(int h, int v);
        int getStatus();
};

extern GigaDisplayShieldClass GigaDisplayShield;
extern USBCVideoClass USBCVideo;