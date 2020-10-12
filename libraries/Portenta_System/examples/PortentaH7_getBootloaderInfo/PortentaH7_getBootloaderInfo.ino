uint8_t* bootloader_data = (uint8_t*)(0x801F000);

void setup() {  
  Serial.begin(115200);
  while (!Serial) {}
 
  Serial.println("Magic Number (validation): " + String(bootloader_data[0], HEX));
  Serial.println("Bootloader version: " + String(bootloader_data[1]));
  Serial.println("Clock source: " + getClockSource(bootloader_data[2]));
  Serial.println("USB Speed: " + getUSBSpeed(bootloader_data[3]));
  Serial.println("Has Ethernet: " + String(bootloader_data[4] == 1 ? "Yes" : "No"));
  Serial.println("Has WiFi module: " + String(bootloader_data[5] == 1 ? "Yes" : "No"));
  Serial.println("RAM size: " + String(bootloader_data[6]) + " MB");
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

void loop() {  
  delay(1000);
}
