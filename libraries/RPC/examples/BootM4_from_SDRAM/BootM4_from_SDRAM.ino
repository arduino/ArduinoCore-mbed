#ifdef CORE_CM7

#include "SDRAM.h"
#include "QSPIFBlockDevice.h"
#include "MBRBlockDevice.h"
#include "FATFileSystem.h"
#include "PluggableUSBMSD.h"

QSPIFBlockDevice root;
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
    Serial.println("Please run WiFiFirmwareUpdater once");
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

#elif defined(CORE_CM4)

#warning "Compiling a Blink, change the delay or the colour and then copy the .bin into PORTENTA mass storage as fw.bin"

int led = LEDB;
int delay_ms = 1000;

void setup() {
  pinMode(led, OUTPUT);
}

void loop() {
  digitalWrite(led, HIGH);
  delay(delay_ms);
  digitalWrite(led, LOW);
  delay(delay_ms);
}

#else

#error Wrong target selected

#endif