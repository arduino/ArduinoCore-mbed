#include "Arduino.h"
#include "Wire.h"
#include "anx7625_public.h"

struct device {
    void* stuff;
};

struct i2c_client {
    int addr;
    struct device dev;
};

mbed::I2C i2c(I2C_SDA , I2C_SCL); 

inline int i2c_smbus_write_byte_data(struct i2c_client * client, uint8_t command, uint8_t value) {

    char cmd[2];
    cmd[0] = command;
    cmd[1] = value;
    return i2c.write(client->addr << 1, cmd, 2);

    Wire.beginTransmission(client->addr);
    Wire.write(command);
    Wire.write(value);
    return Wire.endTransmission(true);
}

inline int i2c_smbus_write_i2c_block_data(struct i2c_client * client, uint8_t command, size_t len, uint8_t* buf) {

    char cmd[len +1];
    cmd[0] = command;
    memcpy(&cmd[1], buf, len);
    return i2c.write(client->addr << 1, cmd, len + 1);

    Wire.beginTransmission(client->addr);
    Wire.write(command);
    Wire.write(buf, len);
    return Wire.endTransmission(true);
}

inline int i2c_smbus_read_i2c_block_data(struct i2c_client * client, uint8_t reg_addr, size_t len, uint8_t* buf) {

    char cmd[len];
    cmd[0] = reg_addr;
    i2c.write(client->addr << 1, cmd, 1);
    return i2c.read(client->addr << 1, cmd, len);

    Wire.beginTransmission(client->addr);
    Wire.write(reg_addr);
    int ret = Wire.endTransmission(false);
    if (ret != 0) {
        return -1;
    }

    Wire.requestFrom(client->addr, len);
    size_t i = 0;
    while (Wire.available() && i < len) {
        buf[i++] = Wire.read();
    }
    return i;
}

inline int i2c_smbus_read_byte_data(struct i2c_client * client, uint8_t reg_addr) {

    char cmd[1];
    cmd[0] = reg_addr;
    i2c.write(client->addr << 1, cmd, 1);
    i2c.read(client->addr << 1, cmd, 1);
    return cmd[0];

    Wire.beginTransmission(client->addr);
    Wire.write(reg_addr);
    int ret = Wire.endTransmission(false);
    if (ret != 0) {
        return -1;
    }

    Wire.requestFrom(client->addr, 1);
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
        if ((timeout_us > 0) && (millis() > timeout)) { \
            (val) = op(addr); \
            break; \
        } \
        if (sleep_us) \
            delayMicroseconds(sleep_us); \
    } \
    (cond) ? 0 : -ETIMEDOUT; \
})

#define DRM_DEV_DEBUG_DRIVER(dev, ...)      printf(__VA_ARGS__)
#define DRM_ERROR                           printf
#define DRM_DEV_ERROR(dev, ...)             printf(__VA_ARGS__)
#define DRM_INFO                            printf
#define pr_err                              printf
#define pr_info                             printf

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

inline struct gpio_desc* devm_gpiod_get(struct device *dev, char* name, int status) {
    if (strcmp(name, "reset_n") == 0) {
        return new gpio_desc(PJ_3);
    }
    if (strcmp(name, "power_en") == 0) {
        return new gpio_desc(PK_2);
    }
    if (strcmp(name, "cbl_det") == 0) {
        return new gpio_desc(PK_3);
    }
    if (strcmp(name, "intp") == 0) {
        return new gpio_desc(PK_4);
    }
    return NULL;
}

inline PinName desc_to_gpio(struct gpio_desc* gpio) {
    return gpio->name;
}

inline void gpiod_set_value_cansleep(struct gpio_desc* gpio, int value) {
    digitalWrite(gpio->name, value);
}

inline int gpiod_get_value_cansleep(struct gpio_desc* gpio) {
    return digitalRead(gpio->name);
}

inline void usleep_range(int low, int high) {
    delayMicroseconds(low);
}

#define u8          uint8_t
#define u16         uint16_t
#define u32         uint32_t

typedef struct { volatile int counter; } atomic_t;

#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr)-(size_t)(&((type *)0)->member)))

struct workqueue_struct;

struct work_struct;
typedef void (*work_func_t)(struct work_struct *work);

struct work_struct {
    work_func_t func;
};

#define IS_ERR(x)               (0)
#define PTR_ERR(x)              (-1)
#define PTR_ERR_OR_ZERO(x)      (0)

#define BIT(nr)         (1 << (nr))

enum display_flags {
    DISPLAY_FLAGS_HSYNC_LOW     = BIT(0),
    DISPLAY_FLAGS_HSYNC_HIGH    = BIT(1),
    DISPLAY_FLAGS_VSYNC_LOW     = BIT(2),
    DISPLAY_FLAGS_VSYNC_HIGH    = BIT(3),

    /* data enable flag */
    DISPLAY_FLAGS_DE_LOW        = BIT(4),
    DISPLAY_FLAGS_DE_HIGH       = BIT(5),
    /* drive data on pos. edge */
    DISPLAY_FLAGS_PIXDATA_POSEDGE   = BIT(6),
    /* drive data on neg. edge */
    DISPLAY_FLAGS_PIXDATA_NEGEDGE   = BIT(7),
    DISPLAY_FLAGS_INTERLACED    = BIT(8),
    DISPLAY_FLAGS_DOUBLESCAN    = BIT(9),
    DISPLAY_FLAGS_DOUBLECLK     = BIT(10),
    /* drive sync on pos. edge */
    DISPLAY_FLAGS_SYNC_POSEDGE  = BIT(11),
    /* drive sync on neg. edge */
    DISPLAY_FLAGS_SYNC_NEGEDGE  = BIT(12),
};

struct timing_entry {
    u32 min;
    u32 typ;
    u32 max;
};

struct display_timing {
    struct timing_entry pixelclock;

    struct timing_entry hactive;        /* hor. active video */
    struct timing_entry hfront_porch;   /* hor. front porch */
    struct timing_entry hback_porch;    /* hor. back porch */
    struct timing_entry hsync_len;      /* hor. sync len */

    struct timing_entry vactive;        /* ver. active video */
    struct timing_entry vfront_porch;   /* ver. front porch */
    struct timing_entry vback_porch;    /* ver. back porch */
    struct timing_entry vsync_len;      /* ver. sync len */

    enum display_flags flags;       /* display flags */
};

struct notifier_block;

typedef int (*notifier_fn_t)(struct notifier_block *nb,
            unsigned long action, void *data);

struct notifier_block {
    notifier_fn_t notifier_call;
    struct notifier_block *next;
    int priority;
};

// gcd == greatest common divisor
inline int gcd(int n1, int n2)
{
    if (n2 != 0)
       return gcd(n2, n1%n2);
    else 
       return n1;
}

#define drm_edid_is_valid(x)    true

static inline bool schedule_work(struct work_struct *work) {
    return true;
};

#define NOTIFY_DONE     0x0000      /* Don't care */
#define NOTIFY_OK       0x0001      /* Suits me */
#define NOTIFY_STOP_MASK    0x8000      /* Don't call further */
#define NOTIFY_BAD      (NOTIFY_STOP_MASK|0x0002)

#define LOCK_PREFIX

#define atomic_read(x)  (*(x.counter))

#define atomic_set(x, y) (*(x.counter) = y)

static __always_inline void arch_atomic_inc(atomic_t *v)
{
    asm volatile(LOCK_PREFIX "incl %0"
             : "+m" (v->counter) :: "memory");
}
#define atomic_inc arch_atomic_inc

/**
 * arch_atomic_dec - decrement atomic variable
 * @v: pointer of type atomic_t
 *
 * Atomically decrements @v by 1.
 */
static __always_inline void arch_atomic_dec(atomic_t *v)
{
    asm volatile(LOCK_PREFIX "decl %0"
             : "+m" (v->counter) :: "memory");
}
#define atomic_dec arch_atomic_dec

static rtos::Mutex _mut;

void mutex_lock(struct mutex* mut) {
    _mut.lock();
}

void mutex_init(struct mutex* mut) {
}

void mutex_unlock(struct mutex* mut) {
    _mut.unlock();
}

#define GPIOD_OUT_LOW       OUTPUT
#define GPIOD_IN            INPUT

#define EXTCON_DISP_DP      44

struct extcon_dev {
    void* stuff;
};

int extcon_get_state(struct extcon_dev *edev, unsigned int id) {
    // TODO: changeme!!
    return EXTCON_DISP_DP;
}

#define INIT_WORK   

#define devm_extcon_register_notifier   1

#define queue_work

/*
*/

inline void drm_helper_hpd_irq_event(struct device* dev) {

};

enum irqreturn {
    IRQ_HANDLED         = (1 << 0),
    IRQ_WAKE_THREAD     = (1 << 1),
};
typedef enum irqreturn irqreturn_t;

static int drm_panel_unprepare(struct drm_panel *panel) {
    return 0;
};

static int drm_panel_prepare(struct drm_panel *panel) {
    return 0;
};

static int drm_connector_update_edid_property(struct drm_connector *connector, struct edid * edid_raw) {
    return 0;
}

static int drm_add_edid_modes(struct drm_connector *connector, struct edid * edid_raw) {
    return 1;
}

enum drm_connector_status {
    connector_status_connected = 1,
    connector_status_disconnected = 2,
    connector_status_unknown = 3,
};

struct drm_connector_funcs {
  int (* dpms) (struct drm_connector *connector, int mode);
  void (* reset) (struct drm_connector *connector);
  enum drm_connector_status (* detect) (struct drm_connector *connector,bool force);
  void (* force) (struct drm_connector *connector);
  int (* fill_modes) (struct drm_connector *connector, uint32_t max_width, uint32_t max_height);
  int (* set_property) (struct drm_connector *connector, struct drm_property *property,uint64_t val);
  void (* destroy) (struct drm_connector *connector);
  struct drm_connector_state *(* atomic_duplicate_state) (struct drm_connector *connector);
  void (* atomic_destroy_state) (struct drm_connector *connector,struct drm_connector_state *state);
  int (* atomic_set_property) (struct drm_connector *connector,struct drm_connector_state *state,struct drm_property *property,uint64_t val);
  int (* atomic_get_property) (struct drm_connector *connector,const struct drm_connector_state *state,struct drm_property *property,uint64_t *val);
};


#define DP_AUX_MAX_PAYLOAD_BYTES    16
#define DP_AUX_I2C_WRITE        0x0
#define DP_AUX_I2C_READ         0x1
#define DP_AUX_I2C_WRITE_STATUS_UPDATE  0x2
#define DP_AUX_I2C_MOT          0x4
#define DP_AUX_NATIVE_WRITE     0x8
#define DP_AUX_NATIVE_READ      0x9
#define DP_AUX_NATIVE_REPLY_ACK     (0x0 << 0)
#define DP_AUX_NATIVE_REPLY_NACK    (0x1 << 0)
#define DP_AUX_NATIVE_REPLY_DEFER   (0x2 << 0)
#define DP_AUX_NATIVE_REPLY_MASK    (0x3 << 0)
#define DP_AUX_I2C_REPLY_ACK        (0x0 << 2)
#define DP_AUX_I2C_REPLY_NACK       (0x1 << 2)
#define DP_AUX_I2C_REPLY_DEFER      (0x2 << 2)
#define DP_AUX_I2C_REPLY_MASK       (0x3 << 2)
