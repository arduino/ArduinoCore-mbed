#include "Arduino.h"
#include "WiFi.h"

using namespace mbed;

#include "OdinWiFiInterface.h"

#if 0
static OdinWiFiInterface* wifi_if;
void* cb() {
	wifi_if = new OdinWiFiInterface();
	return wifi_if;
}

arduino::WiFiClass WiFi(cb);
#else
static OdinWiFiInterface wifi_if;
arduino::WiFiClass WiFi(&wifi_if);
#endif