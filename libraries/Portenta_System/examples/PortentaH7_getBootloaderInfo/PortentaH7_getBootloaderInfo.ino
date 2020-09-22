uint8_t* bootloader_data = (uint8_t*)(0x801F000);

void setup() {  
  Serial.begin(115200);
  while (!Serial) {}
 
  Serial.println("Validation: " + String(bootloader_data[0], HEX));
  Serial.println("BL version: " + String(bootloader_data[1]));
  Serial.println("Clock source: " + String(bootloader_data[2]));
  Serial.println("USB Speed: " + String(bootloader_data[3]));
  Serial.println("Ethernet: " + String(bootloader_data[4]));
  Serial.println("Wifi: " + String(bootloader_data[5]));
  Serial.println("RAM size: " + String(bootloader_data[6]));
  Serial.println("QSPI size: " + String(bootloader_data[7]));
  Serial.println("Video: " + String(bootloader_data[8]));
  Serial.println("Crypto: " + String(bootloader_data[9]));
}

void loop() {  
  delay(1000);
}
