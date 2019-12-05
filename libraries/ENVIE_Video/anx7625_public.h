// public APIs
#include "Arduino.h"

#define DRM_DISPLAY_MODE_LEN    32

struct drm_bridge {
    void* encoder;
    struct device* dev;
};

enum drm_mode_status {
  MODE_OK,
  MODE_HSYNC,
  MODE_VSYNC,
  MODE_H_ILLEGAL,
  MODE_V_ILLEGAL,
  MODE_BAD_WIDTH,
  MODE_NOMODE,
  MODE_NO_INTERLACE,
  MODE_NO_DBLESCAN,
  MODE_NO_VSCAN,
  MODE_MEM,
  MODE_VIRTUAL_X,
  MODE_VIRTUAL_Y,
  MODE_MEM_VIRT,
  MODE_NOCLOCK,
  MODE_CLOCK_HIGH,
  MODE_CLOCK_LOW,
  MODE_CLOCK_RANGE,
  MODE_BAD_HVALUE,
  MODE_BAD_VVALUE,
  MODE_BAD_VSCAN,
  MODE_HSYNC_NARROW,
  MODE_HSYNC_WIDE,
  MODE_HBLANK_NARROW,
  MODE_HBLANK_WIDE,
  MODE_VSYNC_NARROW,
  MODE_VSYNC_WIDE,
  MODE_VBLANK_NARROW,
  MODE_VBLANK_WIDE,
  MODE_PANEL,
  MODE_INTERLACE_WIDTH,
  MODE_ONE_WIDTH,
  MODE_ONE_HEIGHT,
  MODE_ONE_SIZE,
  MODE_NO_REDUCED,
  MODE_NO_STEREO,
  MODE_STALE,
  MODE_BAD,
  MODE_ERROR
};  

struct drm_display_mode {
  char name[DRM_DISPLAY_MODE_LEN];
  enum drm_mode_status status;
  unsigned int type;
  int clock;
  int hdisplay;
  int hsync_start;
  int hsync_end;
  int htotal;
  int hskew;
  int vdisplay;
  int vsync_start;
  int vsync_end;
  int vtotal;
  int vscan;
  unsigned int flags;
  int width_mm;
  int height_mm;
  int crtc_clock;
  int crtc_hdisplay;
  int crtc_hblank_start;
  int crtc_hblank_end;
  int crtc_hsync_start;
  int crtc_hsync_end;
  int crtc_htotal;
  int crtc_hskew;
  int crtc_vdisplay;
  int crtc_vblank_start;
  int crtc_vblank_end;
  int crtc_vsync_start;
  int crtc_vsync_end;
  int crtc_vtotal;
  int private_flags;
  int vrefresh;
  int hsync;
};

struct gpio_desc {
    gpio_desc(PinName _name) : name(_name) {}
    PinName name;
};

struct drm_connector {
    int polled;
    struct device* dev;
};

struct drm_dp_link {
    void* stuff;
};

struct drm_dp_aux {
    void* stuff;
};

struct drm_dp_aux_msg {
    unsigned int address;
    uint8_t request;
    uint8_t reply;
    uint8_t * buffer;
    size_t size;
};

struct mutex {
};

/*
struct anx7625_platform_data {
	struct gpio_desc *gpiod_intp;
	struct gpio_desc *gpiod_cdet;
	struct gpio_desc *gpiod_p_on;
	struct gpio_desc *gpiod_reset;

	mbed::InterruptIn* cdet_irq;
	mbed::InterruptIn* intp_irq;
};

*/

struct device {
    void* platform_data;
};

struct i2c_client {
    int addr;
    struct device dev;
};

struct anx7625 {
	struct drm_dp_aux aux;
	struct drm_bridge bridge;
	struct i2c_client *client;
//	struct anx7625_platform_data pdata;
	struct mutex lock;
	int mode_idx;

	uint16_t chipid;

	bool powered;
	bool enabled;
	int connected;
	bool hpd_status;
	uint8_t sys_sta_bak;

	unsigned char last_read_DevAddr;
};

int anx7625_i2c_probe(struct i2c_client *anx7625);
int anx7625_i2c_remove(struct i2c_client *anx7625);
ssize_t anx7625_debug(const char *buf);
void anx7625_start_dp(void);