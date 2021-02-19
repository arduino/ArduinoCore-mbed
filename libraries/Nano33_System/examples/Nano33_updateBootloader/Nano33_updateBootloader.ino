#include "FlashIAP.h"
#include "MBR.h"
#include "SoftDevice.h"
#include "bootloader.h"

#define MBR_ADDR              (0x0)
#define SOFTDEVICE_ADDR       (0xA0000)
#define BOOTLOADER_ADDR       (0xE0000)
#define SOFTDEVICE_INFO_ADDR  (0xFFFF0)

const unsigned int magic = 0x5f27a93d;

mbed::FlashIAP flash;

void setup() {  
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Do you want to update the bootloader? Y/[n]");
  
  bool confirmation = false;
  while (confirmation == false) {
    if (Serial.available()) {
      char choice = Serial.read();
      switch (choice) {
        case 'y':
        case 'Y':
          flash.init();
          Serial.println("Flasing MBR...");
          applyUpdate(MBR_ADDR);
          Serial.println("Storing SoftDevice length info at 0xF0000...");
          writeSoftDeviceLen(SOFTDEVICE_INFO_ADDR);
          Serial.println("Flasing SoftDevice...");
          applyUpdate(SOFTDEVICE_ADDR);
          Serial.println("Flasing bootloader...");
          applyUpdate(BOOTLOADER_ADDR);
          flash.deinit();
          Serial.println("Bootloader update complete. You may now disconnect the board.");
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


void applyUpdate(uint32_t address) {
  long len = 0;

  const uint32_t page_size = flash.get_page_size();
  Serial.print("Page size: ");
  Serial.println(page_size);
  char *page_buffer = new char[page_size];
  uint32_t addr = address;
  if (addr == MBR_ADDR) {
    len = MBR_bin_len;
  } else if (addr == SOFTDEVICE_ADDR) {
    len = Softdevice_bin_len;
  } else if (addr == BOOTLOADER_ADDR) {
    len = nano33_bootloader_hex_len;
  }
  uint32_t sector_size = flash.get_sector_size(addr);
  Serial.print("Sector size: ");
  Serial.println(sector_size);
  uint32_t next_sector = addr + sector_size;
  bool sector_erased = false;
  size_t pages_flashed = 0;
  uint32_t percent_done = 0;
  
  uint32_t pointer = 0;

  while (true) {
    
    if (pointer >= len) {
      break;
    }

    flash.erase(addr + pointer, sector_size);

    if ((len - pointer) < sector_size) {
      sector_size = len - pointer;
    }

    // Program page
    if (addr == MBR_ADDR) {
      flash.program(&MBR_bin[pointer], addr + pointer, sector_size);
    } else if (addr == SOFTDEVICE_ADDR) {
      flash.program(&Softdevice_bin[pointer], addr + pointer, sector_size);
    } else if (addr == BOOTLOADER_ADDR) {
      flash.program(&nano33_bootloader_hex[pointer], addr + pointer, sector_size);
    }
    
    pointer = pointer + sector_size;

    uint32_t percent_done = pointer * 100 / len;
    Serial.println("Flashed " + String(percent_done) + "%");

  }

  delete[] page_buffer;
}

void writeSoftDeviceLen(uint32_t address) {
  uint32_t sd_addr = SOFTDEVICE_ADDR;
  flash.erase(address, 16);
  //Write flag to let Bootloader understand that SoftDevice binary must be moved
  flash.program(&magic, address, 4);
  //Write address where the SoftDevice binary has been written
  flash.program(&sd_addr, address + 4, 4);
  //Write SoftDevice binary length
  flash.program(&Softdevice_bin_len, address + 8, 4);
}

void loop() {
  delay(1000);
}
