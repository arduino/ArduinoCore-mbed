/*
  SE05X Private Key

  This sketch uses the SE05X to generate a new EC NIST P-256 keypair
  and store it with id 999, then the public key is printed in DER format.

  Circuit:
   - Portenta
   - Nicla Vision
*/

#include <SE05X.h>

void printBufferHex(const byte input[], size_t inputLength) {
  for (int i = 0; i < inputLength; i++) {
    Serial.print(input[i] >> 4, HEX);
    Serial.print(input[i] & 0x0f, HEX);
  }
  Serial.println();
}

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!SE05X.begin()) {
    Serial.println("Failed to communicate with SE05X!");
    while (1);
  }

  const int KeyId = 999;
  byte derBuf[256];
  size_t derSize;

  SE05X.generatePrivateKey(KeyId, derBuf, sizeof(derBuf), &derSize);
  printBufferHex(derBuf, derSize);
}

void loop() {

}
