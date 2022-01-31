#include <GPS.h>
#include <GSM.h>

REDIRECT_STDOUT_TO(Serial);

#include "arduino_secrets.h"
char pin[]      = SECRET_PIN;
char apn[]      = SECRET_APN;
char username[] = SECRET_USERNAME;
char pass[]     = SECRET_PASSWORD;

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("Starting Carrier Network registration");
  if(!GSM.begin(pin, apn, username, pass, CATNB)){
    Serial.println("The board was not able to register to the network...");
    // do nothing forevermore:
    while(1);
  }
  Serial.println("\nEnable GNSS Engine...");
  // GPS.begin() start and eanble the GNSS engine
  GPS.begin();
  Serial.println("\nGNSS Engine enabled...");
}

void loop() {
  if(GPS.available()){
    Serial.print((char) GPS.read());
    delay(1);
  }
  // After geting valid packet GPS.end() can be used to stop and
  // disable the GNSS engine
  // GPS.end();
}
