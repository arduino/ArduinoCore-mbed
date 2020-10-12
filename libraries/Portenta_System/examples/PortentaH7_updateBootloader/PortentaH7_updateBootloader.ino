#include "FlashIAP.h"
#include "bootloader.h"

#ifdef CORE_CM4
  #define Serial Serial1
#endif

#define BOOTLOADER_ADDR   (0x8000000)
mbed::FlashIAP flash;

uint8_t* bootloader_data = (uint8_t*)(0x801F000);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("Validation: " + String(bootloader_data[0], HEX));
  Serial.println("BL version: " + String(bootloader_data[1]));
  Serial.println("Clock source: " + String(bootloader_data[2]));
  Serial.println("USB Speed: " + String(bootloader_data[3]));
  Serial.println("Ethernet: " + String(bootloader_data[4]));
  Serial.println("Wifi: " + String(bootloader_data[5]));
  Serial.println("RAM size: " + String(bootloader_data[6]));
  Serial.println("QSPI size: " + String(bootloader_data[7]));
  Serial.println("Video: " + String(bootloader_data[8]));
  Serial.println("Crypto: " + String(bootloader_data[9]));

  if (bootloader_data[1] < 15) {
    Serial.println("New bootloader version available");
  }
  Serial.println("Update bootloader? Y/[n]");
  bool confirmation = false;
  while (confirmation == false) {
    if (Serial.available()) {
      char choice = Serial.read();
      switch (choice) {
        case 'y':
        case 'Y':
          applyUpdate(BOOTLOADER_ADDR);
          confirmation = true;
          break;
        case 'n':
        case 'N':
          confirmation = true;
          break;
        default:
          continue;
      }
    }
  }
}

void applyUpdate(uint32_t address)
{
  long len = envie_bootloader_mbed_bin_len;

  flash.init();

  const uint32_t page_size = flash.get_page_size();
  char *page_buffer = new char[page_size];
  uint32_t addr = address;
  uint32_t next_sector = addr + flash.get_sector_size(addr);
  bool sector_erased = false;
  size_t pages_flashed = 0;
  uint32_t percent_done = 0;

  while (true) {

    if (page_size * pages_flashed > len) {
      break;
    }

    // Erase this page if it hasn't been erased
    if (!sector_erased) {
      flash.erase(addr, flash.get_sector_size(addr));
      sector_erased = true;
    }

    // Program page
    flash.program(&envie_bootloader_mbed_bin[page_size * pages_flashed], addr, page_size);

    addr += page_size;
    if (addr >= next_sector) {
      next_sector = addr + flash.get_sector_size(addr);
      sector_erased = false;
    }

    if (++pages_flashed % 3 == 0) {
      uint32_t percent_done_new = page_size * pages_flashed * 100 / len;
      if (percent_done != percent_done_new) {
        percent_done = percent_done_new;
        Serial.println("Flashed " + String(percent_done) + "%");
      }
    }
  }
  Serial.println("Flashed 100%");

  delete[] page_buffer;

  flash.deinit();
  Serial.println("Bootloader update complete. You may now disconnect the board.");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(1000);
}
