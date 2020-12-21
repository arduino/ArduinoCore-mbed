#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include "wiced_resource.h"
#include "certificates.h"

#ifndef CORE_CM7  
  #error Update the WiFi firmware by uploading the sketch to the M7 core instead of the M4 core.
#endif

QSPIFBlockDevice root(PD_11, PD_12, PF_7, PD_13,  PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
mbed::MBRBlockDevice wifi_data(&root, 1);
mbed::MBRBlockDevice ota_data(&root, 2);
mbed::FATFileSystem wifi_data_fs("wlan");
mbed::FATFileSystem ota_data_fs("fs");

long getFileSize(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    return size;
}

void setup() {

  Serial.begin(115200);
  while (!Serial);

  mbed::MBRBlockDevice::partition(&root, 1, 0x0B, 0, 1024 * 1024);
  mbed::MBRBlockDevice::partition(&root, 2, 0x0B, 1024 * 1024, 14 * 1024 * 1024);
  // use space from 15.5MB to 16 MB for another fw, memory mapped

  int err =  wifi_data_fs.mount(&wifi_data);
  if (err) {
    // Reformat if we can't mount the filesystem
    // this should only happen on the first boot
    Serial.println("No filesystem containing the WiFi firmware was found.");
    Serial.println("Usually that means that the WiFi firmware has not been installed yet"
                  " or was overwritten with another firmware.\n");
    Serial.println("Formatting the filsystem to install the firmware and certificates...\n");
    err = wifi_data_fs.reformat(&wifi_data);
  }

  err =  ota_data_fs.mount(&ota_data);
  if (err) {
    // Reformat if we can't mount the filesystem
    // this should only happen on the first boot
    Serial.println("No filesystem for OTA firmware was found, creating");
    err = ota_data_fs.reformat(&ota_data);
  }

  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir("/wlan")) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      Serial.println("Searching for WiFi firmware file " + String(ent->d_name) + " ...");
      String fullname = "/wlan/" + String(ent->d_name);
      if (fullname == "/wlan/4343WA1.BIN") {
        Serial.println("A WiFi firmware is already installed. "
                       "Do you want to install the firmware anyway? Y/[n]");
        while (1) {
          if (Serial.available()) {
            int c = Serial.read();
            if (c == 'Y' || c == 'y') {
              wifi_data_fs.reformat(&wifi_data);
              break;
            }
            if (c == 'N' || c == 'n') {
              return;
            }
          }
        }
      }
    }
    closedir (dir);
  }

  extern const unsigned char wifi_firmware_image_data[];
  extern const resource_hnd_t wifi_firmware_image;
  FILE* fp = fopen("/wlan/4343WA1.BIN", "wb");
  int ret = fwrite(wifi_firmware_image_data, 421098, 1, fp);
  fclose(fp);

  root.program(wifi_firmware_image_data, 15 * 1024 * 1024 + 1024 * 512, 421098);

  fp = fopen("/wlan/cacert.pem", "wb");
  ret = fwrite(cacert_pem, cacert_pem_len, 1, fp);
  fclose(fp);

  fp = fopen("/wlan/cacert.pem", "rb");
  char buffer[128];
  ret = fread(buffer, 1, 128, fp);
  Serial.write(buffer, ret);
  while (ret == 128) {
    ret = fread(buffer, 1, 128, fp);
    Serial.write(buffer, ret);
  }
  fclose(fp);

  Serial.println("\nFirmware and certificates updated!");
}

void loop() {

}