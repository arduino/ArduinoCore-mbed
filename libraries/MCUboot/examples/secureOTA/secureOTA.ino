/*
  This example shows how to perform an OTA with MCUboot
  using MCUboot library.

  WARNING: The ota binary is signed and encrypted with default keys.
  The example will work only if default keys are flashed within the
  bootloader otherwise MCUboot will refuse to boot the OTA binary

  Circuit:
  - Arduino Portenta H7 board

  This example code is in the public domain.
*/

#include "BlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include <MCUboot.h>
#include <WiFi.h>

static char const SSID[] = "SECRET_SSID";  /* your network SSID (name) */
static char const PASS[] = "SECRET_PASS";  /* your network password (use for WPA, or use as key for WEP) */

static char const OTA_FILE_LOCATION[] = "https://downloads.arduino.cc/ota/OTA_Usage_Portenta.ino.PORTENTA_H7_M7.MCUboot.slot";

static const int MCUBOOT_SLOT_SIZE = 0x1E0000;

bool applyUpdate = false;
bool confirmUpdate = false;

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(9600);
  while(!Serial);

  if (WiFi.status() == WL_NO_SHIELD)
    return;

  int status = WL_IDLE_STATUS;
  while (status != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SSID);
    status = WiFi.begin(SSID, PASS);
    delay(3000);
  }
  Serial.println("Connected to wifi");

  Serial.println("Are you ready to apply a new update? Y/[n]");
  applyUpdate = waitResponse();

  if (applyUpdate) {

    // Mount filesystem and download the OTA file
    mbed::BlockDevice * raw = mbed::BlockDevice::get_default_instance();
    mbed::MBRBlockDevice * mbr = new mbed::MBRBlockDevice(raw, 2);
    int err = mbr->init();
    if (err < 0) {
      Serial.print("Error initializing Block Device ");
      Serial.println(err);
      return;
    }
    
    mbed::FATFileSystem * fs = new mbed::FATFileSystem("ota");
    err = fs->mount(mbr);
    if (err < 0) {
      Serial.print("Error mounting filesystem ");
      Serial.println(err);
      return;
    }
    
    Serial.println("Downloading update file");
    err = WiFi.download((char*)OTA_FILE_LOCATION, "/ota/update.bin", true);
    if (err < 0) {
      Serial.print("Error downloading file ");
      Serial.println(err);
      return;
    }

    // OTA file is not padded to reduce download time,
    // but MCUboot needs update file to be padded 
    FILE* update_file = fopen("/ota/update.bin", "rb+");
    fseek(update_file, 0, SEEK_END);
    int fsize = ftell(update_file);

    Serial.print("File update.bin size ");
    Serial.println( fsize );

    if (fsize < MCUBOOT_SLOT_SIZE) {
      const char buffer[1] = {0xFF};

      Serial.println("Padding update file");
      printProgress(fsize, MCUBOOT_SLOT_SIZE, 10, true);
      while (fsize < MCUBOOT_SLOT_SIZE) {
        int ret = fwrite(buffer, 1, 1, update_file);
        if (ret != 1) {
          Serial.println("Error writing update file");
          break;
        }
        fsize += 1;
        printProgress(fsize, MCUBOOT_SLOT_SIZE, 10, false);
      }
    }
    Serial.println("Flashed 100%");

    fseek(update_file, 0, SEEK_END);
    Serial.print("File update.bin size ");
    Serial.println( fsize );

    if(fsize != 0x1E0000) {
      Serial.print("Error padding file ");
      return;
    }

    fclose(update_file);
    fs->unmount();

    // Is it possible to pre-confirm padded OTA file to prevent rollback
    Serial.println("Do you want to make the update permanent? Y/[n]");
    confirmUpdate = waitResponse();

    // Set update pending and image OK flags
    MCUboot::applyUpdate(confirmUpdate);
    Serial.println("Done, waiting reset");
  } else {
    Serial.println("No update pending. It's now safe to reboot or disconnect your board.");
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

// the loop function runs over and over again forever
void loop() {
  // wait 100ms
  delay(100);
}
