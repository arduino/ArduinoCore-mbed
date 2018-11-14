#include "Arduino.h"
#include "WiFi.h"

using namespace mbed;

#include "OdinWiFiInterface.h"

static OdinWiFiInterface wifi_if;
arduino::WiFiClass WiFi(&wifi_if);
