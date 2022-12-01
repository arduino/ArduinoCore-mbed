uint8_t* bootloader_data = (uint8_t*)(0x801F000);
uint8_t* bootloader_identification = (uint8_t*)(0x80002F0);

#if __has_include("portenta_info.h")
#include "portenta_info.h"
#define GET_PORTENTA_OTP_BOARD_INFO
PortentaBoardInfo *info;
PortentaBoardInfo* boardInfo();
#endif

#if __has_include("opta_info.h")
#include "opta_info.h"
#define GET_OPTA_OTP_BOARD_INFO
OptaBoardInfo *info;
OptaBoardInfo* boardInfo();
#endif

void setup() {  
  Serial.begin(115200);
  while (!Serial) {}

  uint8_t currentBootloaderVersion = bootloader_data[1];
  String currentBootloaderIdentifier = String(bootloader_identification, 15);

  if(!currentBootloaderIdentifier.equals("MCUboot Arduino")) {
    currentBootloaderIdentifier = "Arduino loader";
  }

  Serial.println(currentBootloaderIdentifier);
  Serial.println("Magic Number (validation): " + String(bootloader_data[0], HEX));
  Serial.println("Bootloader version: " + String(bootloader_data[1]));

#if defined(GET_PORTENTA_OTP_BOARD_INFO)
  printPortentaSecureInfo();
#elif defined(GET_OPTA_OTP_BOARD_INFO)
  printOptaSecureInfo();
#else
  printBootloaderInfo();
#endif
}

#if defined(GET_PORTENTA_OTP_BOARD_INFO)
void printPortentaSecureInfo() {
  info = boardInfo();
  if (info->magic == 0xB5) {
    Serial.println("Secure info version: " + String(info->version));
    Serial.println("USB Speed: " + String(info->_board_functionalities.usb_high_speed == 1 ? "USB 2.0/Hi-Speed (480 Mbps)" : "USB 1.1/Full-Speed (12 Mbps)"));
    Serial.println("Has Ethernet: " + String(info->_board_functionalities.ethernet == 1 ? "Yes" : "No"));
    Serial.println("Has WiFi module: " + String(info->_board_functionalities.wifi == 1 ? "Yes" : "No"));
    Serial.println("Has Video output: " + String(info->_board_functionalities.video == 1 ? "Yes" : "No"));
    Serial.println("Has SE050 crypto: " + String(info->_board_functionalities.nxp_crypto == 1 ? "Yes" : "No"));
    Serial.println("Has ATECC crypto: " + String(info->_board_functionalities.mchp_crypto == 1 ? "Yes" : "No"));
    Serial.println("RAM size: " + getRAMSize(info->external_ram_size));
    Serial.println("QSPI size: " + String(info->external_flash_size) + " MB");
    Serial.println("Secure board revision: " + String(info->revision >> 8) + "." + String(info->revision & 0xFF));
    Serial.println("Secure carrier identification: " + String(info->carrier >> 8) + "." + String(info->revision & 0xFF));
    Serial.println("Secure vid: 0x" + String(info->vid, HEX));
    Serial.println("Secure pid: 0x" + String(info->pid, HEX));
    Serial.println("Secure mac: " + String(info->mac_address[0], HEX) + ":" + String(info->mac_address[1], HEX) + ":" +
                                    String(info->mac_address[2], HEX) + ":" + String(info->mac_address[3], HEX) + ":" +
                                    String(info->mac_address[4], HEX) + ":" + String(info->mac_address[5], HEX));
  } else {
    Serial.println("No secure info available");
    printBootloaderInfo();
  }
}
#endif

#if defined(GET_OPTA_OTP_BOARD_INFO)
void printOptaSecureInfo() {
  info = boardInfo();
  if (info->magic == 0xB5) {
    Serial.println("Secure info version: " + String(info->version));
    Serial.println("Has Ethernet: " + String(info->_board_functionalities.ethernet == 1 ? "Yes" : "No"));
    Serial.println("Has WiFi module: " + String(info->_board_functionalities.wifi == 1 ? "Yes" : "No"));
    Serial.println("Has RS485: " + String(info->_board_functionalities.rs485 == 1 ? "Yes" : "No"));
    Serial.println("QSPI size: " + String(info->external_flash_size) + " MB");
    Serial.println("Secure board revision: " + String(info->revision >> 8) + "." + String(info->revision & 0xFF));
    Serial.println("Secure vid: 0x" + String(info->vid, HEX));
    Serial.println("Secure pid: 0x" + String(info->pid, HEX));
    Serial.println("Secure mac: " + String(info->mac_address[0], HEX) + ":" + String(info->mac_address[1], HEX) + ":" +
                                    String(info->mac_address[2], HEX) + ":" + String(info->mac_address[3], HEX) + ":" +
                                    String(info->mac_address[4], HEX) + ":" + String(info->mac_address[5], HEX));
  } else {
    Serial.println("No secure info available");
    printBootloaderInfo();
  }
}
#endif

void printBootloaderInfo() {
  Serial.println("Clock source: " + getClockSource(bootloader_data[2]));
  Serial.println("USB Speed: " + getUSBSpeed(bootloader_data[3]));
  Serial.println("Has Ethernet: " + String(bootloader_data[4] == 1 ? "Yes" : "No"));
  Serial.println("Has WiFi module: " + String(bootloader_data[5] == 1 ? "Yes" : "No"));
  Serial.println("RAM size: " + getRAMSize(bootloader_data[6]));
  Serial.println("QSPI size: " + String(bootloader_data[7]) + " MB");
  Serial.println("Has Video output: " + String(bootloader_data[8] == 1 ? "Yes" : "No"));
  Serial.println("Has Crypto chip: " + String(bootloader_data[9] == 1 ? "Yes" : "No"));
}

String getUSBSpeed(uint8_t flag) {
  switch (flag){
  case 1:
    return "USB 2.0/Hi-Speed (480 Mbps)";
  case 2:
    return "USB 1.1/Full-Speed (12 Mbps)";
  default:
    return "N/A";
  }
}

String getClockSource(uint8_t flag) {
  switch (flag){
  case 0x8:
    return "External oscillator";
  case 0x4:
    return "External crystal";
  case 0x2: 
    return "Internal clock"; 
  default:
    return "N/A";
  }
}

String getRAMSize(uint8_t flag) {
  if (flag == 0) {
    return "N/A";
  }
  return (String(flag) + "MB");
}

void loop() {  
  delay(1000);
}
