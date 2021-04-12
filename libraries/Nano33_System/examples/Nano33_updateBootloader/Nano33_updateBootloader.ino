/*
 *  This sketch will allow to support SoftDevices on Nano 33 BLE.
 *  To be able to support them, the bootloader needs to be updated.
 *  Before uploading this sketch to the board, follow this procedure:
 *
 *  - Convert your SoftDevice binary to a SoftDevice.h .
 *    The nRF5-SDK website provides a SoftDevice.hex, so run the following commands:
 *
 *      objcopy --input-target=ihex --output-target=binary --gap-fill 0xff SoftDevice.hex SoftDevice.bin
 *      xxd -i SoftDevice.bin > SoftDevice.h
 *
 *  - Copy the content of the generated header file to SoftDevice.h
 *
 *  - Upload this sketch to a Nano33BLE and make your selection through the Serial monitor:
 *    you can choose wheter to update only the SoftDevice or the full bootloader.
 *    After completion, the board will reboot and enter the bootloader mode.
 *
 *  - Now you can upload your sketch.
 *    You can upload a sketch that uses the SoftDevice at 0x26000, using the following bossac command
 *
 *      /path/to/bossac -d --port=yourPort --offset=0x16000 -U -i -e -w /path/to/sketch.bin -R
 *
 *    Or you can still upload a standard sketch from the IDE at 0x10000. This will of course overwrite the SoftDevice.
 *    So if you want to run a SoftDevice-related sketch, always remember to upload this sketch before.
 *
 */

#include "FlashIAP.h"
#include "SoftDevice.h"
#include "bootloader.h"
#include "nrf_nvmc.h"

#define MBR_ADDR              (0x0)
#define SOFTDEVICE_ADDR       (0xA0000)
#define BOOTLOADER_ADDR       (0xE0000)
#define SOFTDEVICE_INFO_ADDR  (0xFF000)
#define UICR_BOOT_ADDR        (0x10001014)

const unsigned int magic = 0x5f27a93d;

mbed::FlashIAP flash;

void setup() {  
  Serial.begin(115200);
  while (!Serial) {}
  Serial.println("Do you want to update only the SoftDevice? Y/[n]");
  Serial.println("Yes: upload only the binary of the DoftDevice");
  Serial.println("No : upload the SoftDevice binary and update the bootloader to support SoftDevices");
  
  bool confirmation = false;
  while (confirmation == false) {
    if (Serial.available()) {
      char choice = Serial.read();
      switch (choice) {
        case 'y':
        case 'Y':
          flash.init();
          Serial.println("Storing SoftDevice length info at 0xFF000...");
          writeSoftDeviceLen(SOFTDEVICE_INFO_ADDR);
          Serial.println("Flasing SoftDevice...");
          applyUpdate(SOFTDEVICE_ADDR);
          flash.deinit();
          Serial.println("SoftDevice update complete! The board is restarting...");
          confirmation = true;
          NVIC_SystemReset();
          break;
        case 'n':
        case 'N':
          flash.init();
          Serial.println("Flasing MBR...");
          applyUpdate(MBR_ADDR);
          Serial.println("Storing SoftDevice length info at 0xFF000...");
          writeSoftDeviceLen(SOFTDEVICE_INFO_ADDR);
          Serial.println("Flasing SoftDevice...");
          applyUpdate(SOFTDEVICE_ADDR);
          Serial.println("Flasing bootloader...");
          applyUpdate(BOOTLOADER_ADDR);
          Serial.println("Write in UICR memory the address of the new bootloader...");
          nrf_nvmc_write_word(UICR_BOOT_ADDR, BOOTLOADER_ADDR);
          flash.deinit();
          Serial.println("SoftDevice + Bootloader update complete. The board is restarting...");
          confirmation = true;
          NVIC_SystemReset();
          break;
        default:
          continue;
      }
    }
  }
}


void applyUpdate(uint32_t address) {
  long len = 0;
  
  uint32_t bin_pointer = 0;
  uint32_t flash_pointer = 0;

  const uint32_t page_size = flash.get_page_size();
  //Serial.print("Page size: ");
  //Serial.println(page_size);
  char *page_buffer = new char[page_size];
  uint32_t addr = address;
  if (addr == MBR_ADDR) {
    bin_pointer = 0;
    len = 4096;
  } else if (addr == SOFTDEVICE_ADDR) {
    //Skip the MBR
    bin_pointer = 4096;
    len = Softdevice_bin_len - 4096;
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
      flash.program(&Softdevice_bin[bin_pointer], addr + flash_pointer, sector_size);
    } else if (addr == SOFTDEVICE_ADDR) {
      flash.program(&Softdevice_bin[bin_pointer], addr + flash_pointer, sector_size);
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
