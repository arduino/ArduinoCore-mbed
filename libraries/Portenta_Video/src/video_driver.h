#ifndef VIDEO_DRIVER_H
#define VIDEO_DRIVER_H

#include "lvgl.h"

void portenta_init_video();
void giga_init_video(bool landscape = true);

//@TODO: Remove
void my_disp_flush(lv_disp_drv_t * disp, const lv_area_t * area, lv_color_t * color_p);

#endif // VIDEO_DRIVER_H