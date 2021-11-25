#include <GSM.h>
#include <MicroNMEA.h>

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

char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

void loop() {

  delay(1000);

  // Output GPS information from previous second
  Serial.print("Valid fix: ");
  Serial.println(nmea.isValid() ? "yes" : "no");

  if (nmea.isValid()) {

    Serial.print("Nav. system: ");
    if (nmea.getNavSystem())
      Serial.println(nmea.getNavSystem());
    else
      Serial.println("none");

    Serial.print("Num. satellites: ");
    Serial.println(nmea.getNumSatellites());

    Serial.print("HDOP: ");
    Serial.println(nmea.getHDOP() / 10., 1);

    Serial.print("Date/time: ");
    Serial.print(nmea.getYear());
    Serial.print('-');
    Serial.print(int(nmea.getMonth()));
    Serial.print('-');
    Serial.print(int(nmea.getDay()));
    Serial.print('T');
    Serial.print(int(nmea.getHour()));
    Serial.print(':');
    Serial.print(int(nmea.getMinute()));
    Serial.print(':');
    Serial.println(int(nmea.getSecond()));

    long latitude_mdeg = nmea.getLatitude();
    long longitude_mdeg = nmea.getLongitude();
    Serial.print("Latitude (deg): ");
    Serial.println(latitude_mdeg / 1000000., 6);

    Serial.print("Longitude (deg): ");
    Serial.println(longitude_mdeg / 1000000., 6);

    long alt;
    Serial.print("Altitude (m): ");
    if (nmea.getAltitude(alt))
      Serial.println(alt / 1000., 3);
    else
      Serial.println("not available");

    Serial.print("Speed: ");
    Serial.println(nmea.getSpeed() / 1000., 3);
    Serial.print("Course: ");
    Serial.println(nmea.getCourse() / 1000., 3);
  }
}

void mycallback(char * output)
{
  for (size_t i = 0; i < strlen(output); i++) {
    nmea.process(output[i]);
  }
  nmea.process('\0');
  //Serial.println(output);
}
