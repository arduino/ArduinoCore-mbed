#include "SDRAM.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include "PluggableUSBMSD.h"

QSPIFBlockDevice root(PD_11, PD_12, PF_7, PD_13,  PF_10, PG_6, QSPIF_POLARITY_MODE_1, 40000000);
mbed::MBRBlockDevice ota_data(&root, 2);
mbed::FATFileSystem ota_data_fs("fs");

void USBMSD::begin()
{
}

USBMSD MassStorage(&root);

long getFileSize(FILE *fp) {
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  return size;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  while (!Serial);

  SDRAM.begin(0);

  int err = ota_data_fs.mount(&ota_data);
  if (err) {
    Serial.println("Please run PortentaWiFiFirmwareUpdater once");
    while (1) {
      delay(10000);
    }
  }

  // Copy M4 firmware to SDRAM
  FILE* fw = fopen("/fs/fw.bin", "r");
  if (fw == NULL) {
    Serial.println("Please copy a firmware for M4 core in the PORTENTA mass storage");
    Serial.println("When done, please unmount the mass storage and reset the board");
    MassStorage.begin();
    while (1) {
      delay(10000);
    }
  }
  fread((uint8_t*)CM4_BINARY_START, getFileSize(fw), 1, fw);
  fclose(fw);

  bootM4();
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}
