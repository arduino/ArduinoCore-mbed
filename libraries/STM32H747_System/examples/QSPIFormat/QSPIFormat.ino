#include "BlockDevice.h"
#include "MBRBlockDevice.h"
#include "LittleFileSystem.h"
#include "FATFileSystem.h"

#ifndef CORE_CM7  
  #error Format QSPI flash by uploading the sketch to the M7 core instead of the M4 core.
#endif

using namespace mbed;

BlockDevice* root = BlockDevice::get_default_instance();
MBRBlockDevice wifi_data(root, 1);
MBRBlockDevice ota_data(root, 2);
MBRBlockDevice kvstore_data(root, 3);
MBRBlockDevice user_data(root, 4);
FATFileSystem wifi_data_fs("wlan");
FATFileSystem ota_data_fs("fs");
FileSystem * user_data_fs;

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

void setup() {

  Serial.begin(115200);
  while (!Serial);

  Serial.println("\nWARNING! Running the sketch all the content of the QSPI flash will be erased.");
  Serial.println("The following partitions will be created:");
  Serial.println("Partition 1: WiFi firmware and certificates 1MB");
  Serial.println("Partition 2: OTA 5MB");
  Serial.println("Partition 3: Provisioning KVStore 1MB");
  Serial.println("Partition 4: User data / OPTA PLC runtime 7MB"),
  Serial.println("Do you want to proceed? Y/[n]");

  if (true == waitResponse()) {
    MBRBlockDevice::partition(root, 1, 0x0B, 0, 1 * 1024 * 1024);
    MBRBlockDevice::partition(root, 2, 0x0B, 1 * 1024 * 1024,  6 * 1024 * 1024);
    MBRBlockDevice::partition(root, 3, 0x0B, 6 * 1024 * 1024,  7 * 1024 * 1024);
    MBRBlockDevice::partition(root, 4, 0x0B, 7 * 1024 * 1024, 14 * 1024 * 1024);
    // use space from 15.5MB to 16 MB for another fw, memory mapped

    int err = wifi_data_fs.reformat(&wifi_data);
    if (err) {
      Serial.println("Error formatting WiFi partition");
      return;
    }
  
    err = ota_data_fs.reformat(&ota_data);
    if (err) {
      Serial.println("Error formatting OTA partition");
      return;
    }

    Serial.println("\nDo you want to use LittleFS to format user data partition? Y/[n]");
    Serial.println("If No, FatFS will be used to format user partition.");

    if (true == waitResponse()) {
      Serial.println("Formatting user partition with LittleFS.");
      user_data_fs = new mbed::LittleFileSystem("user");
    } else {
      Serial.println("Formatting user partition with FatFS.");
      user_data_fs = new mbed::FATFileSystem("user");
    }

    err = user_data_fs->reformat(&user_data);
    if (err) {
      Serial.println("Error formatting user partition");
      return;
    }
    Serial.println("\nQSPI Flash formatted!");
  }

  Serial.println("It's now safe to reboot or disconnect your board.");
}

void loop() {

}
