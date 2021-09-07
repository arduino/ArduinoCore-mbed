#include <GSM.h>

REDIRECT_STDOUT_TO(Serial);

#include "arduino_secrets.h"
char pin[]      = SECRET_PIN;
char apn[]      = SECRET_APN;
char username[] = SECRET_USERNAME;
char pass[]     = SECRET_PASSWORD;

void mycallback(char * output);

void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  //GSM.debug(Serial);
  Serial.println("\nStarting connection to server...");
  GSM.begin(pin, apn, username, pass, CATNB);
  Serial.println("\nStarting connection ...");

  Serial.println("\nEnable GNSS Engine...");
  GSM.beginGNSS(mycallback);
  GSM.startGNSS();
}

void loop() {
//    GSM.startGNSS();
//    delay(10000);
//    GSM.stopGNSS();
}
void mycallback(char * output)
{
  Serial.println(output);
}
