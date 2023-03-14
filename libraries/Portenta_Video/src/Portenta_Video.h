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
    private:
    };  

}
#endif /* PORTENTA_VIDEO_H */