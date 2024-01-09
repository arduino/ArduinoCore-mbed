#include "H7DisplayShield.h"

#include "Arduino.h"
#include "anx7625.h"
#include "st7701.h"
extern "C" {
#include "video_modes.h"
}

int GigaDisplayShieldClass::init(int edidmode) {
    //Init LCD Controller
    st7701_init((enum edid_modes) edidmode);

    return 0;
}

int GigaDisplayShieldClass::getEdidMode(int h, int v) {
    return EDID_MODE_480x800_60Hz;
}

int USBCVideoClass::init(int edidmode) {
    struct edid recognized_edid;
    int err_code = 0;

    memset(&recognized_edid, 0, sizeof(recognized_edid));

    //Initialization of ANX7625
    err_code = anx7625_init(0);
    if(err_code < 0) {
        return err_code;
    }

    //Checking HDMI plug event
    anx7625_wait_hpd_event(0);

    //Read EDID
    anx7625_dp_get_edid(0, &recognized_edid);

    //DSI Configuration
    anx7625_dp_start(0, &recognized_edid, (enum edid_modes) edidmode);

    return 0;
}

int USBCVideoClass::getEdidMode(int h, int v) {
    int edidmode = video_modes_get_edid(h, v);

    return edidmode;
}

GigaDisplayShieldClass GigaDisplayShield;
USBCVideoClass USBCVideo;