#include "FlashIAP.h"
#if defined(ARDUINO_PORTENTA_H7_M7)
#include "portenta_bootloader.h"
#include "portenta_lite_bootloader.h"
#include "portenta_lite_connected_bootloader.h"
#elif defined(ARDUINO_NICLA_VISION)
#include "nicla_vision_bootloader.h"
#endif

#ifndef CORE_CM7  
  #error Update the bootloader by uploading the sketch to the M7 core instead of the M4 core.
#endif

#define BOOTLOADER_ADDR   (0x8000000)
mbed::FlashIAP flash;

uint32_t bootloader_data_offset = 0x1F000;
uint8_t* bootloader_data = (uint8_t*)(BOOTLOADER_ADDR + bootloader_data_offset);

bool video_available = false;
bool wifi_available = false;

void setup() {  
  Serial.begin(115200);
  while (!Serial) {}

  uint8_t currentBootloaderVersion = bootloader_data[1];
  uint8_t availableBootloaderVersion = (bootloader_mbed_bin + bootloader_data_offset)[1];

  Serial.println("Magic Number (validation): " + String(bootloader_data[0], HEX));
  Serial.println("Bootloader version: " + String(currentBootloaderVersion));
  Serial.println("Clock source: " + getClockSource(bootloader_data[2]));
  Serial.println("USB Speed: " + getUSBSpeed(bootloader_data[3]));
  Serial.println("Has Ethernet: " + String(bootloader_data[4] == 1 ? "Yes" : "No"));
  Serial.println("Has WiFi module: " + String(bootloader_data[5] == 1 ? "Yes" : "No"));
  Serial.println("RAM size: " + String(bootloader_data[6]) + " MB");
  Serial.println("QSPI size: " + String(bootloader_data[7]) + " MB");
  Serial.println("Has Video output: " + String(bootloader_data[8] == 1 ? "Yes" : "No"));
  Serial.println("Has Crypto chip: " + String(bootloader_data[9] == 1 ? "Yes" : "No"));

  video_available = bootloader_data[8];
  wifi_available = bootloader_data[5];

  if (availableBootloaderVersion > currentBootloaderVersion) {
    Serial.print("\nA new bootloader version is available: v" + String(availableBootloaderVersion));
    Serial.println(" (Your version: v" + String(currentBootloaderVersion) + ")");
    Serial.println("Do you want to update the bootloader? Y/[n]");
  } else if(availableBootloaderVersion < currentBootloaderVersion){ 
    Serial.println("\nA newer bootloader version is already installed: v" + String(currentBootloaderVersion));    
    Serial.println("Do you want to downgrade the bootloader to v" + String(availableBootloaderVersion) + "? Y/[n]");
  } else {
    Serial.println("\nThe latest version of the bootloader is already installed (v" + String(currentBootloaderVersion) + ").");
    Serial.println("Do you want to update the bootloader anyway? Y/[n]");
  }
  
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

String getUSBSpeed(uint8_t flag) {
  switch (flag){
  case 1:
    return "USB 2.0/Hi-Speed (480 Mbps)";
  case 2:
    return "USB 1.1/Full-Speed (12 Mbps)";
  default:
    return "N/A";
  }
}

String getClockSource(uint8_t flag) {
  switch (flag){
  case 0x8:
    return "External oscillator";
  case 0x4:
    return "External crystal";
  case 0x2: 
    return "Internal clock"; 
  default:
    return "N/A";
  }
}

void applyUpdate(uint32_t address) {
  long len = bootloader_mbed_bin_len;
#if defined(ARDUINO_PORTENTA_H7_M7)
  if (!video_available) {
    if (wifi_available) {
      len = bootloader_mbed_lite_connected_bin_len;
    } else {
      len = bootloader_mbed_lite_bin_len;
    }
  }
#endif

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
#if defined(ARDUINO_PORTENTA_H7_M7)
  if (video_available) {
    flash.program(&bootloader_mbed_bin[page_size * pages_flashed], addr, page_size);
  } else {
    if (wifi_available) {
      flash.program(&bootloader_mbed_lite_connected_bin[page_size * pages_flashed], addr, page_size);
    } else {
      flash.program(&bootloader_mbed_lite_bin[page_size * pages_flashed], addr, page_size);
    }
  }
#else
  flash.program(&bootloader_mbed_bin[page_size * pages_flashed], addr, page_size);
#endif

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
  delay(1000);
}
