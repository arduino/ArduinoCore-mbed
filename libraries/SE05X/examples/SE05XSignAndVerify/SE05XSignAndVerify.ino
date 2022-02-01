/*
  SE05X SignAndVerify

  This sketch uses the SE05X to generate a new EC NIST P-256 keypair
  and store it with id 999, then input buffer SHA256 is signed with the private
  key and verified with the public key.

  Circuit:
   - Portenta
   - Nicla Vision
*/

#include <SE05X.h>

const byte input[64] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f
};

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

  // print the input
  Serial.print("Input is:                     ");
  printBufferHex(input, sizeof(input));

  //calculate the input SHA256
  byte sha256[256];
  size_t sha256Len;
  SE05X.SHA256(input, sizeof(input), sha256, sizeof(sha256), &sha256Len);
  Serial.print("Input SHA256 is:              ");
  printBufferHex(sha256, sha256Len);

  // calculate the signature, input MUST be SHA256
  byte signature[256];
  size_t signatureLen;
  SE05X.Sign(KeyId, sha256, sha256Len, signature, sizeof(signature), &signatureLen);

  // print the signature
  Serial.print("Signature using KeyId ");
  Serial.print(KeyId);
  Serial.print(" is: ");
  printBufferHex(signature, signatureLen);

  Serial.println();

  // To make the signature verifcation fail, uncomment the next line:
  //  signature[0] = 0x00;

  // validate the signature
  if (SE05X.Verify(KeyId, sha256, sha256Len, signature, signatureLen)) {
    Serial.println("Verified signature successfully :D");
  } else {
    Serial.println("oh no! failed to verify signature :(");
  }
}

void loop() {

}
