/*
 *  This sketch writes a Soft Device to Nano 33 BLE flash.
 *  Uploade this sketch ONLY AFTER having updated the Nano 33 BLE bootloader with 'Nano33_updateBootloader.ino' sketch.
 *
 *  To use a Soft Device different from the one provided here, follow this procedure:
 *
 *  - Convert your SoftDevice binary to a SoftDevice.h .
 *    The nRF5-SDK website provides a SoftDevice.hex, so run the following commands:
 *
 *      objcopy --input-target=ihex --output-target=binary --gap-fill 0xff SoftDevice.hex SoftDevice.bin
 *      xxd -i SoftDevice.bin > SoftDevice.h
 *
 *  - Copy the content of the generated header file to SoftDevice.h
 *
 *  - Upload this sketch to a Nano 33 BLE to write the SoftDevice in flash.
 *    After completion, the board will reboot and enter the bootloader mode.
 *
 *  - Now you can upload any sketch that uses the SoftDevice at 0x26000, using the following bossac command
 *
 *      /path/to/bossac -d --port=yourPort --offset=0x16000 -U -i -e -w /path/to/sketch.bin -R
 *
 *    Or you can still upload a standard sketch from the IDE at 0x10000. This will of course overwrite the SoftDevice.
 *    So if you want to run a SoftDevice-related sketch, always remember to upload this sketch before.
 *    
 *    NOTE: this sketch defines bootloader info (BOOTLOADER_CRC and BOOTLOADER_SIZE) which will need to be updated
 *    if a new version of the bootloader is released.
 *
 */

#include "FlashIAP.h"
#include "SoftDevice.h"

#define SOFTDEVICE_ADDR       (0xA0000)
#define BOOTLOADER_ADDR       (0xE0000)
#define SOFTDEVICE_INFO_ADDR  (0xFF000)

#define BOOTLOADER_CRC         0x5E797B91
#define BOOTLOADER_SIZE        35984

const unsigned int magic = 0x5f27a93d;

mbed::FlashIAP flash;

void setup() {  
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("*** Nano33_updateSoftDevice sketch ***");
  Serial.println("Checking if the bootloader supports Soft Devices...");

  flash.init();

  //Compute bootloader CRC32
  uint32_t crc32 = getBootloaderCrc();

  if (crc32 != BOOTLOADER_CRC) {
    Serial.println("Bootloader needs to be updated! Please upload the sketch 'Nano33_updateBootloader'.");
    flash.deinit();
    while(1);
  }
  
  Serial.println("Correct booloader found!");
  Serial.println();
  
  Serial.println("Storing SoftDevice length info at 0xFF000...");
  writeSoftDeviceLen(SOFTDEVICE_INFO_ADDR);
  
  Serial.println("Flasing SoftDevice...");
  applyUpdate(SOFTDEVICE_ADDR);

  flash.deinit();

  Serial.println();
  Serial.println("SoftDevice update complete! The board is restarting...");
  NVIC_SystemReset();

}


uint32_t getBootloaderCrc() {
  uint32_t b = 0;
  uint32_t crc = 0xFFFFFFFF;

  int iterations = ceil(BOOTLOADER_SIZE/4);

  int addr = BOOTLOADER_ADDR;

  for (int i=0; i<iterations; i++) {
    //Read 32 bit from flash
    flash.read(&b, addr, sizeof(b));
    //Serial.println(b, HEX);
    //Update crc
    crc = crc ^ b;
    //Update pointer 
    addr = addr + 4;
  }
  return crc;
}


void applyUpdate(uint32_t address) {
  long len = 0;
  
  uint32_t bin_pointer = 0;
  uint32_t flash_pointer = 0;

  const uint32_t page_size = flash.get_page_size();
  char *page_buffer = new char[page_size];
  uint32_t addr = address;
  
  bin_pointer = 4096;
  len = Softdevice_bin_len - 4096;
  
  uint32_t sector_size = flash.get_sector_size(addr);
  uint32_t next_sector = addr + sector_size;
  bool sector_erased = false;
  size_t pages_flashed = 0;
  uint32_t percent_done = 0;

  while (true) {
    
    if (flash_pointer >= len) {
      break;
    }

    flash.erase(addr + flash_pointer, sector_size);

    if ((len - flash_pointer) < sector_size) {
      sector_size = len - flash_pointer;
    }

    // Program page
    flash.program(&Softdevice_bin[bin_pointer], addr + flash_pointer, sector_size);

    Serial.print("Flash Address = ");
    Serial.println(addr + flash_pointer, HEX);
    
    bin_pointer = bin_pointer + sector_size;
    flash_pointer = flash_pointer + sector_size;

    uint32_t percent_done = flash_pointer * 100 / len;
    Serial.println("Flashed " + String(percent_done) + "%");

  }
  Serial.println();

  delete[] page_buffer;
}

void writeSoftDeviceLen(uint32_t address) {
  uint32_t sd_addr = SOFTDEVICE_ADDR;
  flash.erase(address, 16);
  //Write flag to let Bootloader understand that SoftDevice binary must be moved
  flash.program(&magic, address, 4);
  //Write address where the SoftDevice binary has been written
  flash.program(&sd_addr, address + 4, 4);
  //Write SoftDevice binary length
  unsigned int sd_len = Softdevice_bin_len - 4096;
  flash.program(&sd_len, address + 8, 4);
}

void loop() {
  delay(1000);
}
