#include <GPS.h>
#include <GSM.h>
#include <MicroNMEA.h>

#include "arduino_secrets.h"
constexpr auto pin { SECRET_PIN };
constexpr auto apn { SECRET_APN };
constexpr auto username { SECRET_USERNAME };
constexpr auto pass { SECRET_PASSWORD };

char nmeaBuffer[100];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

// Keep track of NMEA string processing
auto nmeaProcessStatus { false };

// Check for valid fix every checkValidInterval ms
constexpr unsigned long checkValidInterval { 5000 };
unsigned long checkValidNow {};

void setup()
{
    Serial.begin(115200);
    for (const auto timeout = millis() + 2500; !Serial && millis() < timeout; delay(250))
        ;

    // GSM.debug(Serial);
    delay(1000);

    Serial.println("Starting Carrier Network registration");
    if (!GSM.begin(pin, apn, username, pass, CATNB)) {
        Serial.println("The board was not able to register to the network...");
        // do nothing forevermore:
        while (1)
            ;
    }
    Serial.println("Enable GNSS Engine...");
    // GPS.begin() start and eanble the GNSS engine
    GPS.begin();
    Serial.println("GNSS Engine enabled...");
    Serial.println("Waiting for a valid fix.");

    checkValidNow = millis();
}

void loop()
{
    while (GPS.available()) {
        char c = GPS.read();
        // process is true when a valid NMEA string has been processed
        nmeaProcessStatus = nmea.process(c);
    }

    if (nmeaProcessStatus && millis() > checkValidNow) {
        checkValidNow = millis() + checkValidInterval;

        // Output GPS information from previous second
        Serial.print("Valid fix: ");
        Serial.println(nmea.isValid() ? "yes" : "no");

        if (!nmea.isValid())
            return;

        String navSystem;
        switch (nmea.getNavSystem()) {
        case 'N':
            navSystem = "GNSS";
            break;
        case 'P':
            navSystem = "GPS";
            break;
        case 'L':
            navSystem = "GLONASS";
            break;
        case 'A':
            navSystem = "Galileo";
            break;
        default:
            break;
        }
        Serial.print("Nav. system: ");
        Serial.println(navSystem);

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
        nmea.clear();
    }
}
