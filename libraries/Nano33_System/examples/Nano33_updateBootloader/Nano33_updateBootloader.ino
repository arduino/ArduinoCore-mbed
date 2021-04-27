/*
 *  This sketch allows to support Soft Devices on Nano 33 BLE.
 *  
 *  To be able to support Soft Devices, the bootloader first needs to be updated.
 *  Upload this sketch on the Nano 33 BLE to download the new bootloader in the flash. 
 *  Then, use 'Nano33_updateSoftDevice.ino' sketch to upload a SoftDevice.
 *  
 *  The new bootloader is fully backwards compatible with standard sketches.
 *
 */

#include "FlashIAP.h"
#include "MBR.h"
#include "bootloader.h"
#include "nrf_nvmc.h"

#define MBR_ADDR              (0x0)
#define BOOTLOADER_ADDR       (0xE0000)
#define UICR_BOOT_ADDR        (0x10001014)

mbed::FlashIAP flash;

void setup() {  
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("*** Nano33_updateBootloader sketch ***");
  Serial.println("This sketch modifies the Nano33 bootloader to support Soft Devices");
  Serial.println();
  
  flash.init();
  
  Serial.println("Flasing MBR...");
  applyUpdate(MBR_ADDR);
  
  Serial.println("Flasing bootloader...");
  applyUpdate(BOOTLOADER_ADDR);

  Serial.print("Bootloader 32bit CRC is: ");
  uint32_t crc32 = getBootloaderCrc();
  Serial.println(crc32, HEX);
  
  Serial.println("Writing in UICR memory the address of the new bootloader...");
  nrf_nvmc_write_word(UICR_BOOT_ADDR, BOOTLOADER_ADDR);
  
  flash.deinit();

  Serial.println();
  Serial.println("Bootloader update successfully completed!");
  Serial.println("Now use the sketch Nano33_updateSoftDevice to flash a Soft Device.");
}


uint32_t getBootloaderCrc() {
  uint32_t mask = 0;
  uint32_t crc = 0xFFFFFFFF;
  uint32_t b = 0;
  uint8_t bootByte = 0;

  int iterations = nano33_bootloader_hex_len;


  for (int i=0; i<iterations; i=i+4) {
    b = 0;
    for (int j=0; j<4; j++) {
      mask = 0;
      bootByte = nano33_bootloader_hex[i+j];
      mask = mask + (uint32_t)bootByte;
      mask = mask << 8*j;
      b = b | mask;
    }
    crc = crc ^ b;
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
  if (addr == MBR_ADDR) {
    bin_pointer = 0;
    len = MBR_bin_len;
  } else if (addr == BOOTLOADER_ADDR) {
    bin_pointer = 0;
    len = nano33_bootloader_hex_len;
  }
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
    if (addr == MBR_ADDR) {
      flash.program(&MBR_bin[bin_pointer], addr + flash_pointer, sector_size);
    } else if (addr == BOOTLOADER_ADDR) {
      flash.program(&nano33_bootloader_hex[bin_pointer], addr + flash_pointer, sector_size);
    }

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


void loop() {
  delay(1000);
}
