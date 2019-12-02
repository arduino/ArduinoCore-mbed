#include "Arduino.h"
#include "Wire.h"

struct i2c_client {
    int address;
};

struct device {
    void* stuff;
};

struct gpio_desc {
    gpio_desc(PinName _name) : name(_name) {}
    PinName name;
};

inline int i2c_smbus_write_byte_data(struct i2c_client * client, uint8_t command, uint8_t value) {
    Wire.beginTransmission(client->address);
    Wire.write(command);
    Wire.write(value);
    return Wire.endTransmission(true);
}

inline int i2c_smbus_read_i2c_block_data(struct i2c_client * client, uint8_t reg_addr, size_t len, uint8_t* buf) {
    Wire.beginTransmission(client->address);
    Wire.write(reg_addr);
    int ret = Wire.endTransmission(false);
    if (ret != 0) {
        return -1;
    }

    Wire.requestFrom(client->address, len);
    size_t i = 0;
    while (Wire.available() && i < len) {
        buf[i++] = Wire.read();
    }
    return i;
}

inline int i2c_smbus_read_byte_data(struct i2c_client * client, uint8_t reg_addr) {
    Wire.beginTransmission(client->address);
    Wire.write(reg_addr);
    int ret = Wire.endTransmission(false);
    if (ret != 0) {
        return -1;
    }

    Wire.requestFrom(client->address, 1);
    if (Wire.available()) {
        return Wire.read();
    }
    return -1;
}

#define readx_poll_timeout(op, addr, val, cond, sleep_us, timeout_us)   \
({ \
    uint32_t timeout = millis() + timeout_us/1000; \
    delayMicroseconds(sleep_us); \
    for (;;) { \
        (val) = op(addr); \
        if (cond) \
            break; \
        if (timeout_us && millis() > timeout) { \
            (val) = op(addr); \
            break; \
        } \
        if (sleep_us) \
            delayMicroseconds(sleep_us); \
    } \
    (cond) ? 0 : -ETIMEDOUT; \
})

#define DRM_DEV_DEBUG_DRIVER(dev, format, ...)    printf(format, __VA_ARGS__)
#define DRM_ERROR   printf

inline struct gpio_desc* devm_gpiod_get_optional(struct device *dev, char* name, int status) {
    if (strcmp(name, "reset") == 0) {
        return new gpio_desc(PJ_3);
    }
    if (strcmp(name, "enable") == 0) {
        return new gpio_desc(PK_2);
    }
    return NULL;
}

inline PinName desc_to_gpio(struct gpio_desc* gpio) {
    return gpio->name;
}

inline void gpiod_set_value(struct gpio_desc* gpio, int value) {
    digitalWrite(gpio->name, value);
}

inline void usleep_range(int low, int high) {
    delayMicroseconds(low);
}

#define u8          uint8_t
#define u32         uint32_t

typedef struct { volatile int counter; } atomic_t;

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr)-(size_t)(&((type *)0)->member)))

enum drm_mode_status {
    MODE_OK,
    MODE_CLOCK_HIGH
};

#define DRM_DISPLAY_MODE_LEN    32

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