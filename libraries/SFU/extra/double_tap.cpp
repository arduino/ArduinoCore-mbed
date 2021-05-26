extern "C" {
    #include "pico.h"
    #include "pico/time.h"
    #include "pico/bootrom.h"
}

#define LED_DEFAULT 6

// Doesn't make any sense for a RAM only binary
#if !PICO_NO_FLASH

static const uint32_t magic_token[] = {
        0xf01681de, 0xbd729b29, 0xd359be7a,
};

static uint32_t __uninitialized_ram(magic_location)[count_of(magic_token)];

// run at initialization time
static void boot_double_tap_check() {
    for (uint i = 0; i < count_of(magic_token); i++) {
        if (magic_location[i] != magic_token[i]) {
            // Arm for 500 ms then disarm and continue booting
            for (i = 0; i < count_of(magic_token); i++) {
                magic_location[i] = magic_token[i];
            }
            busy_wait_us(500000);
            magic_location[0] = 0;
            return;
        }
    }

    magic_location[0] = 0;
    reset_usb_boot(1 << LED_DEFAULT, 0);
}

class DoubleTap {
public:
    DoubleTap() {
        boot_double_tap_check();
    }
};

DoubleTap dt __attribute__ ((init_priority (101)));

#endif