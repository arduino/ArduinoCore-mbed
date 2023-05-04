/*
  This example enables secuity features of the MCUboot bootloader writing encryption
  and signing key into the microcontroller flash memory.

  Once the keys are loaded you need to build and upload the future sketches enabling
  the Security Settings -> "Signing + Encryption" in the tools menu of the IDE. This
  will create an encrypted and signed binary conforming to the MCUboot image format
  using imgtool. See https://docs.mcuboot.com/design.html#image-format for more details
  about image format.

  Writing the keys will also enable MCUboot image swap using a scratch area. This will
  increase the sketch update time after the upload, but also adds the possibility to
  revert to the previous image version if the update is not confirmed.
  See ConfirmSketch example for more details about setting the confirmed flag.

  Circuit:
  - Arduino Portenta H7 board

  This example code is in the public domain.
*/

#include "FlashIAP.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "LittleFileSystem.h"
#include "FATFileSystem.h"
#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_OPTA) || defined(ARDUINO_GIGA)
#include "ecdsa-p256-encrypt-key.h"
#include "ecdsa-p256-signing-key.h"
#else
#error "Security features not available for this board"
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

bool writeKeys = false;
uint32_t bootloader_data_offset = 0x1F000;
uint8_t* bootloader_data = (uint8_t*)(BOOTLOADER_ADDR + bootloader_data_offset);

uint32_t bootloader_identification_offset = 0x2F0;
uint8_t* bootloader_identification = (uint8_t*)(BOOTLOADER_ADDR + bootloader_identification_offset);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  uint8_t currentBootloaderVersion = bootloader_data[1];
  String currentBootloaderIdentifier = String(bootloader_identification, 15);

  if((currentBootloaderVersion > 24) && (currentBootloaderIdentifier.equals("MCUboot Arduino"))) {
    Serial.println("The sketch comes with a set of default keys to evaluate signing and encryption process");
    Serial.println("If you load the keys, you will need to upload the future sketches with Security Settings -> Signing + Encryption.");
    Serial.println("If you select Security Settings -> None, the sketches will not be executed.");
    Serial.println("Do you want to load the keys? Y/[n]");
    if (waitResponse()) {
      Serial.println("\nPlease notice that loading the keys will enable MCUboot Sketch swap. This will increase the sketch update time after the upload.");
      Serial.println("A violet LED will blink until the sketch is ready to run.");
      Serial.println("Do you want to proceed loading the default keys? Y/[n]");
      writeKeys = waitResponse();
    } 
  } else {
    Serial.println("Security features not available for this bootloader version. Please update it using STM32H747_manageBootloader sketch");
  }
  
  if (writeKeys) {
    applyUpdate();
  }
  Serial.println("It's now safe to reboot or disconnect your board.");
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
    Serial.println("Error creating MCUboot files in OTA partition.");
    Serial.println("Run QSPIformat.ino sketch to format the QSPI flash and fix the issue.");
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

void applyUpdate() {
  flash.init();
  setupMCUBootOTAData();
  flash.program(&enc_priv_key, ENCRYPT_KEY_ADDR, ENCRYPT_KEY_SIZE);
  flash.program(&ecdsa_pub_key, SIGNING_KEY_ADDR, SIGNING_KEY_SIZE);
  flash.deinit();
  Serial.println("\nSecurity features enabled. It's now safe to reboot or disconnect your board.");
}

void loop() {
  delay(1000);
}
