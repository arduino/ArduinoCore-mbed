#include <GPS.h>
#include <GSM.h>

#include "arduino_secrets.h"
char pin[]      = SECRET_PIN;
char apn[]      = SECRET_APN;
char username[] = SECRET_USERNAME;
char pass[]     = SECRET_PASSWORD;

void setup() {

#if defined(ARDUINO_EDGE_CONTROL)
  // Power ON MKR2
  pinMode(ON_MKR2, OUTPUT);
  digitalWrite(ON_MKR2, HIGH);
#endif

  Serial.begin(115200);
  while (!Serial) {}

  // To enable AT Trace debug uncomment the following lines
  //GSM.trace(Serial);
  //GSM.setTraceLevel(4);

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
  // Print out raw NMEA strings.
  // For parsed output look at the MicroNMEA_integration example.
  if(GPS.available()){
    Serial.print((char) GPS.read());
    delay(1);
  }
  // After geting valid packet GPS.end() can be used to stop and
  // disable the GNSS engine
  // GPS.end();
}
