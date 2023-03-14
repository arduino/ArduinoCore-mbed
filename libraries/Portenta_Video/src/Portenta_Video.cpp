#include "Portenta_Video.h"
#include "Portenta_lvgl.h"

arduino::Portenta_Video::Portenta_Video() {

}

int arduino::Portenta_Video::begin() {
    portenta_init_video();
}
