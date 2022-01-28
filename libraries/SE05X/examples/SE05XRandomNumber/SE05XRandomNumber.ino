/*
  SE05X Random Number

  This sketch uses the SE05X to generate a random number
  every second and print it to the Serial monitor

  Circuit:
   - Portenta
   - Nicla Vision
*/

#include <SE05X.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!SE05X.begin()) {
    Serial.println("Failed to communicate with SE05X!");
    while (1);
  }
}

void loop() {
  Serial.print("Random number = ");
  Serial.println(SE05X.random(65535));

  delay(1000);
}
