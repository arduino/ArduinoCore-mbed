#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include "wiced_resource.h"
#include "certificates.h"

#ifndef CORE_CM7  
  #error Update the WiFi firmware by uploading the sketch to the M7 core instead of the M4 core.
#endif

QSPIFBlockDevice root(QSPI_SO0, QSPI_SO1, QSPI_SO2, QSPI_SO3,  QSPI_SCK, QSPI_CS, QSPIF_POLARITY_MODE_1, 40000000);
mbed::MBRBlockDevice wifi_data(&root, 1);
mbed::FATFileSystem wifi_data_fs("wlan");

long getFileSize(FILE *fp) {
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    return size;
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

void setup() {

  Serial.begin(115200);
  while (!Serial);

  mbed::MBRBlockDevice::partition(&root, 1, 0x0B, 0, 1024 * 1024);
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
  const int file_size = 421098;
  int chunck_size = 1024;
  int byte_count = 0;

  Serial.println("Flashing /wlan/4343WA1.BIN file");
  printProgress(byte_count, file_size, 10, true);
  while (byte_count < file_size) {
    if(byte_count + chunck_size > file_size)
      chunck_size = file_size - byte_count;
    int ret = fwrite(&wifi_firmware_image_data[byte_count], chunck_size, 1, fp);
    if (ret != 1) {
      Serial.println("Error writing firmware data");
      break;
    }
    byte_count += chunck_size;
    printProgress(byte_count, file_size, 10, false);
  }
  fclose(fp);

  chunck_size = 1024;
  byte_count = 0;
  const uint32_t offset = 15 * 1024 * 1024 + 1024 * 512;

  Serial.println("Flashing memory mapped firmware");
  printProgress(byte_count, file_size, 10, true);
  while (byte_count < file_size) {
    if(byte_count + chunck_size > file_size)
      chunck_size = file_size - byte_count;
    int ret = root.program(wifi_firmware_image_data, offset + byte_count, chunck_size);
    if (ret != 0) {
      Serial.println("Error writing firmware data");
      break;
    }
    byte_count += chunck_size;
    printProgress(byte_count, file_size, 10, false);
  }

  chunck_size = 128;
  byte_count = 0;
  fp = fopen("/wlan/cacert.pem", "wb");

  Serial.println("Flashing certificates");
  printProgress(byte_count, cacert_pem_len, 10, true);
  while (byte_count < cacert_pem_len) {
    if(byte_count + chunck_size > cacert_pem_len)
      chunck_size = cacert_pem_len - byte_count;
    int ret = fwrite(&cacert_pem[byte_count], chunck_size, 1 ,fp);
    if (ret != 1) {
      Serial.println("Error writing certificates");
      break;
    }
    byte_count += chunck_size;
    printProgress(byte_count, cacert_pem_len, 10, false);
  }
  fclose(fp);

  fp = fopen("/wlan/cacert.pem", "rb");
  char buffer[128];
  int ret = fread(buffer, 1, 128, fp);
  Serial.write(buffer, ret);
  while (ret == 128) {
    ret = fread(buffer, 1, 128, fp);
    Serial.write(buffer, ret);
  }
  fclose(fp);

  Serial.println("\nFirmware and certificates updated!");
  Serial.println("It's now safe to reboot or disconnect your board.");
}

void loop() {

}
