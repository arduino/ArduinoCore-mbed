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
  //GSM.debug(Serial);
  Serial.println("\nStarting connection to server...");
  GSM.begin(pin, apn, username, pass, CATNB);
  Serial.println("\nEnable GNSS Engine...");
  // GPS.begin() start and eanble the GNSS engine
  GPS.begin();
  Serial.println("\nGNSS Engine enabled...");
}

void loop() {
  //GPS.begin();
  if(GPS.available()){
    Serial.print((char) GPS.read());
    delay(1);
  }
  // GPS.end() stop and disable the GNSS engine
  // GPS.end();
}