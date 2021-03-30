#include "mbed.h"
#include "FlashIAPBlockDevice.h"
#include "FATFileSystem.h"

#define SD_MOUNT_PATH           "ota"
#define FULL_UPDATE_FILE_PATH   "/" SD_MOUNT_PATH "/" MBED_CONF_APP_UPDATE_FILE

#define POST_APPLICATION_ADDR   0x10000

#if !defined(POST_APPLICATION_ADDR)
#error "target.restrict_size must be set for your target in mbed_app.json"
#endif

//Pin order: MOSI, MISO, SCK, CS
//FlashIAPBlockDevice sd(XIP_BASE + 0xF00000, 0x100000);
FlashIAPBlockDevice sd(XIP_BASE + 0x100000, 0x100000);
FATFileSystem fs(SD_MOUNT_PATH);
FlashIAP flash;

void apply_update(FILE *file, uint32_t address);

int main()
{
    FILE *file;
    sd.init();
    int err = fs.mount(&sd);
    if (err != 0) {
        printf("No partition found\r\n");
        goto boot;
    }

    file = fopen(FULL_UPDATE_FILE_PATH, "rb");
    if (file != NULL) {
        printf("Firmware update found\r\n");

        apply_update(file, XIP_BASE + POST_APPLICATION_ADDR);

        fclose(file);
        remove(FULL_UPDATE_FILE_PATH);
    } else {
        printf("No update found to apply\r\n");
    }

    fs.unmount();

boot:
    sd.deinit();

    printf("Starting application\r\n");

    mbed_start_application(XIP_BASE + POST_APPLICATION_ADDR + 0x100);
}

void apply_update(FILE *file, uint32_t address)
{
    fseek(file, 0, SEEK_END);
    // Skip the first POST_APPLICATION_ADDR bytes
    long len = ftell(file) - POST_APPLICATION_ADDR;
    printf("Firmware size is %ld bytes\r\n", len);
    fseek(file, POST_APPLICATION_ADDR, SEEK_SET);

    flash.init();

    const uint32_t page_size = flash.get_page_size();
    char *page_buffer = new char[page_size];
    uint32_t addr = address;
    uint32_t next_sector = addr + flash.get_sector_size(addr);
    bool sector_erased = false;
    size_t pages_flashed = 0;
    uint32_t percent_done = 0;
    while (true) {

        // Read data for this page
        memset(page_buffer, 0, sizeof(char) * page_size);
        int size_read = fread(page_buffer, 1, page_size, file);
        if (size_read <= 0) {
            break;
        }

        // Erase this page if it hasn't been erased
        if (!sector_erased) {
            flash.erase(addr, flash.get_sector_size(addr));
            sector_erased = true;
        }

        // Program page
        flash.program(page_buffer, addr, page_size);

        addr += page_size;
        if (addr >= next_sector) {
            next_sector = addr + flash.get_sector_size(addr);
            sector_erased = false;
        }

        if (++pages_flashed % 3 == 0) {
            uint32_t percent_done_new = ftell(file) * 100 / len;
            if (percent_done != percent_done_new) {
                percent_done = percent_done_new;
                printf("Flashed %3ld%%\r", percent_done);
            }
        }
    }
    printf("Flashed 100%%\r\n");

    delete[] page_buffer;

    flash.deinit();
}
