#include "Arduino.h"
#include "WiFi.h"

using namespace mbed;

#include "ESP8266Interface.h"

static ESP8266Interface wifi_if(PD_8, PD_9);
arduino::WiFiClass WiFi(&wifi_if);
