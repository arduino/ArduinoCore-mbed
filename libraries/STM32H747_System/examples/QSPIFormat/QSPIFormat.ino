#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "LittleFileSystem.h"
#include "FATFileSystem.h"

#ifndef CORE_CM7  
  #error Format QSPI flash by uploading the sketch to the M7 core instead of the M4 core.
#endif


QSPIFBlockDevice root(QSPI_SO0, QSPI_SO1, QSPI_SO2, QSPI_SO3,  QSPI_SCK, QSPI_CS, QSPIF_POLARITY_MODE_1, 40000000);
mbed::MBRBlockDevice wifi_data(&root, 1);
mbed::MBRBlockDevice ota_data(&root, 2);
mbed::MBRBlockDevice kvstore_data(&root, 3);
mbed::MBRBlockDevice user_data(&root, 4);
mbed::FATFileSystem wifi_data_fs("wlan");
mbed::FATFileSystem ota_data_fs("fs");
mbed::FileSystem * user_data_fs;

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

  Serial.println("Available partition schemes:");
  Serial.println("\nPartition scheme 1");
  Serial.println("Partition 1: WiFi firmware and certificates 1MB");
  Serial.println("Partition 2: OTA and user data 12MB");
  Serial.println("Partition 3: Provisioning KVStore 1MB");
  Serial.println("\nPartition scheme 2");
  Serial.println("Partition 1: WiFi firmware and certificates 1MB");
  Serial.println("Partition 2: OTA 5MB");
  Serial.println("Partition 3: Provisioning KVStore 1MB");
  Serial.println("Partition 4: User data 7MB"),
  Serial.println("\nDo you want to use partition scheme 1? Y/[n]");
  Serial.println("If No, partition scheme 2 will be used.");
  bool default_scheme = waitResponse();

  Serial.println("\nWARNING! Running the sketch all the content of the QSPI flash will be erased.");
  Serial.println("Do you want to proceed? Y/[n]");

  if (true == waitResponse()) {
    if (root.init() != QSPIF_BD_ERROR_OK) {
      Serial.println(F("Error: QSPI init failure."));
      return;
    }

    root.erase(0x0, root.get_erase_size());

    mbed::MBRBlockDevice::partition(&root, 1, 0x0B, 0, 1024 * 1024);
    if(default_scheme) {
      mbed::MBRBlockDevice::partition(&root, 2, 0x0B, 1024 * 1024, 13 * 1024 * 1024);
      mbed::MBRBlockDevice::partition(&root, 3, 0x0B, 13 * 1024 * 1024, 14 * 1024 * 1024);
      mbed::MBRBlockDevice::partition(&root, 4, 0x0B, 14 * 1024 * 1024, 14 * 1024 * 1024);
      // use space from 15.5MB to 16 MB for another fw, memory mapped
    } else {
      mbed::MBRBlockDevice::partition(&root, 2, 0x0B, 1024 * 1024, 6 * 1024 * 1024);
      mbed::MBRBlockDevice::partition(&root, 3, 0x0B, 6* 1024 * 1024, 7 * 1024 * 1024);
      mbed::MBRBlockDevice::partition(&root, 4, 0x0B, 7 * 1024 * 1024, 14 * 1024 * 1024);
      // use space from 15.5MB to 16 MB for another fw, memory mapped
    }

    bool reformat = true;

    if(!wifi_data_fs.mount(&wifi_data)) {
      Serial.println("\nPartition 1 already contains a filesystem, do you want to reformat it? Y/[n]");
      wifi_data_fs.unmount();

      reformat = waitResponse();
    }

    if (reformat && wifi_data_fs.reformat(&wifi_data)) {
      Serial.println("Error formatting WiFi partition");
      return;
    }

    reformat = true;
    if(!ota_data_fs.mount(&ota_data)) {
      Serial.println("\nPartition 2 already contains a filesystem, do you want to reformat it? Y/[n]");
      ota_data_fs.unmount();

      reformat = waitResponse();
    }

    if (reformat && ota_data_fs.reformat(&ota_data)) {
      Serial.println("Error formatting OTA partition");
      return;
    }

    if(!default_scheme) {
      Serial.println("\nDo you want to use LittleFS to format user data partition? Y/[n]");
      Serial.println("If No, FatFS will be used to format user partition.");

      if (true == waitResponse()) {
        Serial.println("Formatting user partition with LittleFS.");
        user_data_fs = new mbed::LittleFileSystem("user");
      } else {
        Serial.println("Formatting user partition with FatFS.");
        user_data_fs = new mbed::FATFileSystem("user");
      }

      reformat = true;
      if(!user_data_fs->mount(&user_data)) {
        Serial.println("\nPartition 3 already contains a filesystem, do you want to reformat it? Y/[n]");
        user_data_fs->unmount();

        reformat = waitResponse();
      }

      if (reformat && user_data_fs->reformat(&user_data)) {
        Serial.println("Error formatting user partition");
        return;
      }
    }
    Serial.println("\nQSPI Flash formatted!");
  }

  Serial.println("It's now safe to reboot or disconnect your board.");
}

void loop() {

}
