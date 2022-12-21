/*
  YModem uploader / downloader

  Available commands:

  L -> list files
  S -> list files + sha256 checksum
  F -> format external flash
  R, Y -> send and receive files (not directly usable)

    To send a file, overwriting the existing one if it exists

      ./syntiant-uploader send -m "Y" -w "Y" -p $portName $filename

    To receive a file (will be copied in CWD folder)

      ./syntiant-uploader receive  -m "R$filename" -w "R" -p $portName

  For example: ./syntiant-uploader receive  -m "Raudiodump.mp3" -w "R" -p /dev/ttyACM1
*/

#include "SPIFBlockDevice.h"
#include "LittleFileSystem.h"
#include "sha256.h"
#include "ymodem.h"


SPIFBlockDevice spif(SPI_PSELMOSI0, SPI_PSELMISO0, SPI_PSELSCK0, CS_FLASH, 16000000);

mbed::LittleFileSystem fs("fs");

FILE *file;
mbedtls_sha256_context ctx;

int timeout = 0;

long getFileLen(FILE *file) {
  fseek(file, 0, SEEK_END);
  long len = ftell(file);
  fseek(file, 0, SEEK_SET);
  //Decrement len by 1 to remove the CRC from the count
  return len;
}

uint8_t* sha256(char* filename, uint8_t* output) {

  mbedtls_sha256_init(&ctx);
  uint8_t packet[256];
  int total = 0;

  String name = String("/fs/") + filename;

  file = fopen(name.c_str(), "rb");

  if (file != NULL) {
    Serial.print(getFileLen(file));
    Serial.print("    ");
  }

  mbedtls_sha256_starts(&ctx, 0);
  while (!feof(file)) {
    int howMany = fread(packet, 1, sizeof(packet), file);
    mbedtls_sha256_update(&ctx, packet, howMany);
    total += howMany;
  }
  mbedtls_sha256_finish(&ctx, output);
  fclose(file);

  printSha256Sum(output);

  return output;
}

void printSha256Sum(uint8_t* output) {
  for (int i = 0; i < 32; i++) {
    if (output[i] < 0x10) {
      Serial.print("0");
    }
    Serial.print(output[i], HEX);
  }
  Serial.println();
}

void listFiles(bool shasum = false);

char filename[256] = {'\0'};

void listFiles(bool shasum) {
  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir("/fs")) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
      Serial.println(ent->d_name);
      if (shasum) {
        Serial.print("    ");
        uint8_t output[32];
        sha256(ent->d_name, output);
      }
    }
    closedir (dir);
  } else {
    /* could not open directory */
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  Serial.begin(115200);
  int err = fs.mount(&spif);

  memset(filename, 0, sizeof(filename));
}



// the loop function runs over and over again forever
void loop() {

  uint8_t command = 0xFF;

  if (Serial.available()) {
    command = Serial.read();
  }

  if (command == 'Y') {
    FILE* f = fopen("/fs/temp.bin", "wb");
    while (Serial.available()) {
      Serial.read();
    }
    Serial.print("Y");
    int ret = Ymodem_Receive(f, 1024 * 1024, filename);
    String name = String(filename);
    if (ret > 0 && name != "") {
      name = "/fs/" + name;
      fclose(f);
      ret = rename("/fs/temp.bin", name.c_str());
    }
  }
  if (command == 'R') {
    String filename = Serial.readStringUntil('\r');
    filename.trim();
    String filename_abs = String("/fs/") + filename;
    FILE* f = fopen(filename_abs.c_str(), "rb");
    while (Serial.available()) {
      Serial.read();
    }
    if (f != NULL) {
      Serial.print("R");
      int ret = Ymodem_Transmit((char*)filename.c_str(), getFileLen(f), f);
      fclose(f);
    }
  }
  if (command == 'L') {
    listFiles();
  }
  if (command == 'S') {
    listFiles(true);
  }
  if (command == 'F') {
    fs.reformat(&spif);
  }
  if (command == 0xFF) {
    delay(10);
  }
}
