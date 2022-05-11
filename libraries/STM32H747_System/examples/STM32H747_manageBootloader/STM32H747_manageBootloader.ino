#include "FlashIAP.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "LittleFileSystem.h"
#include "FATFileSystem.h"
#if defined(ARDUINO_PORTENTA_H7_M7)
#include "portenta_bootloader.h"
#include "portenta_lite_bootloader.h"
#include "portenta_lite_connected_bootloader.h"
#include "mcuboot_bootloader.h"
#include "ecdsa-p256-encrypt-key.h"
#include "ecdsa-p256-signing-key.h"
#elif defined(ARDUINO_NICLA_VISION)
#include "nicla_vision_bootloader.h"
#endif

#ifndef CORE_CM7
  #error Update the bootloader by uploading the sketch to the M7 core instead of the M4 core.
#endif

#define BOOTLOADER_ADDR   (0x8000000)
#define SIGNING_KEY_ADDR  (0x8000300)
#define ENCRYPT_KEY_ADDR  (0x8000400)
#define ENCRYPT_KEY_SIZE  (0x0000100)
#define SIGNING_KEY_SIZE  (0x0000100)

mbed::FlashIAP flash;
QSPIFBlockDevice root(QSPI_SO0, QSPI_SO1, QSPI_SO2, QSPI_SO3,  QSPI_SCK, QSPI_CS, QSPIF_POLARITY_MODE_1, 40000000);

bool writeLoader = false;
bool writeKeys = false;
bool video_available = false;
bool wifi_available = false;
bool MCUboot = false;

uint32_t bootloader_data_offset = 0x1F000;
uint8_t* bootloader_data = (uint8_t*)(BOOTLOADER_ADDR + bootloader_data_offset);

uint32_t bootloader_identification_offset = 0x2F0;
uint8_t* bootloader_identification = (uint8_t*)(BOOTLOADER_ADDR + bootloader_identification_offset);

const unsigned char* bootloader_ptr = &bootloader_mbed_bin[0];
long bootloader_len = bootloader_mbed_bin_len;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  uint8_t currentBootloaderVersion = bootloader_data[1];
  String currentBootloaderIdentifier = String(bootloader_identification, 15);

  if(!currentBootloaderIdentifier.equals("MCUboot Arduino")) {
    currentBootloaderIdentifier = "Arduino loader";
  }

  Serial.println(currentBootloaderIdentifier);
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

  video_available = bootloader_data[8];
  wifi_available = bootloader_data[5];

#if defined(ARDUINO_PORTENTA_H7_M7)
  Serial.println("\nDo you want to install/update the default Arduino bootloader? Y/[n]");
  Serial.println("Choosing \"No\", will install/update the MCUboot bootloader.");
  if(!waitResponse()) {
    Serial.println("\nMCUboot has been selected. Do you want to proceed? Y/[n]");
    if (waitResponse()) {
      MCUboot = true;
      bootloader_ptr = &mcuboot_bin[0];
      bootloader_len = mcuboot_bin_len;
    } else {
      Serial.println("\nProceeding with the default Arduino bootloader...");
    }
  }
  if (!MCUboot) {
    bootloader_ptr = &bootloader_mbed_bin[0];
    bootloader_len = bootloader_mbed_bin_len;
    if (!video_available) {
      if (wifi_available) {
        bootloader_ptr = &bootloader_mbed_lite_connected_bin[0];
        bootloader_len = bootloader_mbed_lite_connected_bin_len;
      } else {
        bootloader_ptr = &bootloader_mbed_lite_bin[0];
        bootloader_len = bootloader_mbed_lite_bin_len;
      }
    }
  }
#endif

  uint8_t availableBootloaderVersion = (bootloader_ptr + bootloader_data_offset)[1];
  String availableBootloaderIdentifier = String(bootloader_ptr + bootloader_identification_offset, 15);

  if(!availableBootloaderIdentifier.equals("MCUboot Arduino")) {
    availableBootloaderIdentifier = "Arduino loader";
  }

  if (currentBootloaderIdentifier == availableBootloaderIdentifier) {
    if (bootloader_data[0] != 0xA0) {
      Serial.println("\nA new bootloader version (v" + String(availableBootloaderVersion) + ") is available.");
      Serial.println("Do you want to update the bootloader? Y/[n]");
    } else {
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
    }
  } else {
    Serial.println("\nA different bootloader type is available: v" + String(availableBootloaderVersion));
    Serial.println("Do you want to update the bootloader? Y/[n]");
  }
  writeLoader = waitResponse();

  if (writeLoader) {
    if(availableBootloaderIdentifier.equals("MCUboot Arduino")) {
      setupMCUBootOTAData();

      Serial.println("\nThe bootloader comes with a set of default keys to evaluate signing and encryption process");
      Serial.println("If you load the keys, you will need to upload the future sketches with Security Settings -> Signing + Encryption.");
      Serial.println("If you select Security Settings -> None, the sketches will not be executed.");
      Serial.println("Do you want to load the keys? Y/[n]");
      if (waitResponse()) {
        Serial.println("\nPlease notice that loading the keys will enable MCUboot Sketch swap. This will increase the sketch update time after the upload.");
        Serial.println("A violet LED will blink until the sketch is ready to run.");
        Serial.println("Do you want to proceed loading the default keys? Y/[n]");
      }
      writeKeys = waitResponse();
    }
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

void setupMCUBootOTAData() {
  mbed::MBRBlockDevice ota_data(&root, 2);
  mbed::FATFileSystem ota_data_fs("fs");
  
  int err = ota_data_fs.reformat(&ota_data);
  if (err) {
    Serial.println("Error creating MCUboot files in OTA partition");
  }

  FILE* fp = fopen("/fs/scratch.bin", "wb");
  const int scratch_file_size = 128 * 1024;
  const char buffer[128] = {0xFF};
  int size = 0;

  Serial.println("\nCreating scratch file");
  printProgress(size, scratch_file_size, 10, true);
  while (size < scratch_file_size) {
    int ret = fwrite(buffer, sizeof(buffer), 1, fp);
    if (ret != 1) {
      Serial.println("Error writing scratch file");
      break;
    }
    size += sizeof(buffer);
    printProgress(size, scratch_file_size, 10, false);
  }
  fclose(fp);

  fp = fopen("/fs/update.bin", "wb");
  const int update_file_size = 15 * 128 * 1024;
  size = 0;

  Serial.println("\nCreating update file");
  printProgress(size, update_file_size, 10, true);
  while (size < update_file_size) {
    int ret = fwrite(buffer, sizeof(buffer), 1, fp);
    if (ret != 1) {
      Serial.println("Error writing scratch file");
      break;
    }
    size += sizeof(buffer);
    printProgress(size, update_file_size, 5, false);
  }

  fclose(fp);
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

#if defined(ARDUINO_PORTENTA_H7_M7)
  if (writeKeys) {
    flash.program(&enc_priv_key, ENCRYPT_KEY_ADDR, ENCRYPT_KEY_SIZE);
    flash.program(&ecdsa_pub_key, SIGNING_KEY_ADDR, SIGNING_KEY_SIZE);
  }
#endif

  Serial.println("Flashed 100%");

  delete[] page_buffer;

  flash.deinit();
  Serial.println("\nBootloader update complete. It's now safe to reboot or disconnect your board.");
}

void loop() {
  delay(1000);
}
