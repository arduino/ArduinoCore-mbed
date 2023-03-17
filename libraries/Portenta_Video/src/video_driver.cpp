#include "video_driver.h"
#include "SDRAM.h"
#include "video_modes.h"
#include "anx7625.h"
#include "dsi.h"

struct edid recognized_edid;

void portenta_init_video() {
  int ret = -1;

  //Initialization of ANX7625
  ret = anx7625_init(0);
  if(ret < 0) {
    printf("Cannot continue, anx7625 init failed.\n");
    while(1);
  }

  //Checking HDMI plug event
  anx7625_wait_hpd_event(0);

  //Read EDID
  anx7625_dp_get_edid(0, &recognized_edid);

  //DSI Configuration
  anx7625_dp_start(0, &recognized_edid, EDID_MODE_720x480_60Hz);

  //Configure SDRAM 
  SDRAM.begin(getFramebufferEnd());

  getNextFrameBuffer();
  getNextFrameBuffer();
}

void giga_init_video(bool landscape) {
  // put your setup code here, to run once:
  int ret = -1;

  SDRAM.begin();

  extern struct envie_edid_mode envie_known_modes[];
  struct edid _edid;
  int mode = EDID_MODE_480x800_60Hz;
  struct display_timing dt;
  dt.pixelclock = envie_known_modes[mode].pixel_clock;
  dt.hactive = envie_known_modes[mode].hactive;
  dt.hsync_len = envie_known_modes[mode].hsync_len;
  dt.hback_porch = envie_known_modes[mode].hback_porch;
  dt.hfront_porch = envie_known_modes[mode].hfront_porch;
  dt.vactive = envie_known_modes[mode].vactive;
  dt.vsync_len = envie_known_modes[mode].vsync_len;
  dt.vback_porch = envie_known_modes[mode].vback_porch;
  dt.vfront_porch = envie_known_modes[mode].vfront_porch;
  dt.hpol = envie_known_modes[mode].hpol;
  dt.vpol = envie_known_modes[mode].vpol;

  stm32_dsi_config(0, &_edid, &dt);

  getNextFrameBuffer();
  getNextFrameBuffer();
}