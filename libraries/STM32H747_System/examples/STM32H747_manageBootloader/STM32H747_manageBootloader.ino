#include "FlashIAP.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "LittleFileSystem.h"
#include "FATFileSystem.h"
#if defined(ARDUINO_PORTENTA_H7_M7)
#include "portenta_info.h"
#include "portenta_bootloader.h"
#include "portenta_lite_bootloader.h"
#include "portenta_lite_connected_bootloader.h"
#define GET_OTP_BOARD_INFO
typedef PortentaBoardInfo BoardInfo;
#elif defined(ARDUINO_NICLA_VISION)
#include "nicla_vision_bootloader.h"
#elif defined(ARDUINO_OPTA)
#define GET_OTP_BOARD_INFO
#include "opta_info.h"
typedef OptaBoardInfo BoardInfo;
#include "opta_bootloader.h"
#endif

#ifndef CORE_CM7
  #error Update the bootloader by uploading the sketch to the M7 core instead of the M4 core.
#endif

#define BOOTLOADER_ADDR   (0x8000000)

mbed::FlashIAP flash;
QSPIFBlockDevice root(QSPI_SO0, QSPI_SO1, QSPI_SO2, QSPI_SO3,  QSPI_SCK, QSPI_CS, QSPIF_POLARITY_MODE_1, 40000000);

bool writeLoader = false;
bool video_available = false;
bool wifi_available = false;

uint32_t bootloader_data_offset = 0x1F000;
uint8_t* bootloader_data = (uint8_t*)(BOOTLOADER_ADDR + bootloader_data_offset);

const unsigned char* bootloader_ptr = &bootloader_bin[0];
long bootloader_len = bootloader_bin_len;

uint8_t* boardInfo();

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  uint8_t currentBootloaderVersion = bootloader_data[1];

  Serial.println("Magic Number (validation): " + String(bootloader_data[0], HEX));
  Serial.println("Bootloader version: " + String(currentBootloaderVersion));
  Serial.println("Clock source: " + getClockSource(bootloader_data[2]));
  Serial.println("USB Speed: " + getUSBSpeed(bootloader_data[3]));
  Serial.println("Has Ethernet: " + String(bootloader_data[4] == 1 ? "Yes" : "No"));
  Serial.println("Has WiFi module: " + String(bootloader_data[5] == 1 ? "Yes" : "No"));
  Serial.println("RAM size: " + getRAMSize(bootloader_data[6]));
  Serial.println("QSPI size: " + String(bootloader_data[7]) + " MB");
  Serial.println("Has Video output: " + String(bootloader_data[8] == 1 ? "Yes" : "No"));
  Serial.println("Has Crypto chip: " + String(bootloader_data[9] == 1 ? "Yes" : "No"));

#ifdef GET_OTP_BOARD_INFO
  auto info = *((BoardInfo*)boardInfo());
  if (info.magic == 0xB5) {
    Serial.println("Secure info version: " + String(info.version));
    Serial.println("Secure board revision: " + String(info.revision >> 8) + "." + String(info.revision & 0xFF));
#if defined(ARDUINO_PORTENTA_H7_M7)
    Serial.println("Secure carrier identification: " + String(info.carrier >> 8) + "." + String(info.revision & 0xFF));
#endif
    Serial.println("Secure vid: 0x" + String(info.vid, HEX));
    Serial.println("Secure pid: 0x" + String(info.pid, HEX));
    Serial.println("Secure mac: " + String(info.mac_address[0], HEX) + ":" + String(info.mac_address[1], HEX) + ":" +
                                    String(info.mac_address[2], HEX) + ":" + String(info.mac_address[3], HEX) + ":" +
                                    String(info.mac_address[4], HEX) + ":" + String(info.mac_address[5], HEX));
  }
#endif

  video_available = bootloader_data[8];
  wifi_available = bootloader_data[5];

#if defined(ARDUINO_PORTENTA_H7_M7)
  if (!video_available) {
    if (wifi_available) {
      bootloader_ptr = &bootloader_lite_connected_bin[0];
      bootloader_len = bootloader_lite_connected_bin_len;
    } else {
      bootloader_ptr = &bootloader_lite_bin[0];
      bootloader_len = bootloader_lite_bin_len;
    }
  }
#endif

  uint8_t availableBootloaderVersion = (bootloader_ptr + bootloader_data_offset)[1];

  if (availableBootloaderVersion > currentBootloaderVersion) {
    Serial.print("\nA new bootloader version is available: v" + String(availableBootloaderVersion));
    Serial.println(" (Your version: v" + String(currentBootloaderVersion) + ")");
    Serial.println("Do you want to update the bootloader? Y/[n]");
  } else if (availableBootloaderVersion < currentBootloaderVersion) {
    Serial.println("\nA newer bootloader version is already installed: v" + String(currentBootloaderVersion));
    Serial.println("Do you want to downgrade the bootloader to v" + String(availableBootloaderVersion) + "? Y/[n]");
  } else {
    Serial.println("\nThe latest version of the bootloader is already installed (v" + String(currentBootloaderVersion) + ").");
    Serial.println("Do you want to update the bootloader anyway? Y/[n]");
  }

  writeLoader = waitResponse();

  if (writeLoader) {
    applyUpdate(BOOTLOADER_ADDR);
  } else {
    Serial.println("It's now safe to reboot or disconnect your board.");
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

String getRAMSize(uint8_t flag) {
  if (flag == 0) {
    return "N/A";
  }
  return (String(flag) + "MB");
}

void printProgress(uint32_t offset, uint32_t size, uint32_t threshold, bool reset) {
  static int percent_done = 0;
  if (reset == true) {
    percent_done = 0;
    Serial.println("Flashed " + String(percent_done) + "%");
  } else {
    uint32_t percent_done_new = offset * 100 / size;
    if (percent_done_new >= percent_done + threshold) {
      percent_done = percent_done_new;
      Serial.println("Flashed " + String(percent_done) + "%");
    }
  }
}

bool waitResponse() {
  bool confirmation = false;
  while (confirmation == false) {
    if (Serial.available()) {
      char choice = Serial.read();
      switch (choice) {
        case 'y':
        case 'Y':
          confirmation = true;
          return true;
          break;
        case 'n':
        case 'N':
          confirmation = true;
          return false;
          break;
        default:
          continue;
      }
    }
  }
}

void applyUpdate(uint32_t address) {
  long len = bootloader_len;

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
    flash.program(&bootloader_ptr[page_size * pages_flashed], addr, page_size);

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
  Serial.println("\nBootloader update complete. It's now safe to reboot or disconnect your board.");
}

void loop() {
  delay(1000);
}
