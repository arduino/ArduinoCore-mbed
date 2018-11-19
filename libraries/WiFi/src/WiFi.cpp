#include "WiFi.h"

int arduino::WiFiClass::begin(char* ssid, const char *passphrase) {
    if (_ssid) free(_ssid);

    _ssid = (char*)malloc(33);
    if (!_ssid) {
        //tr_error("Could not allocate ssid buffer");
        return WL_CONNECT_FAILED;
    }

    if (wifi_if == NULL) {
       wifi_if = (WiFiInterface*)cb();
    }

    // too long? break it off
    if (strlen(ssid) > 32) ssid[32] = 0;
    memcpy(_ssid, ssid, 33);

    Serial.println("Connecting to " + String(_ssid));
    nsapi_error_t ret = wifi_if->connect(ssid, passphrase, NSAPI_SECURITY_WPA2);

    if (ret == NSAPI_ERROR_OK) {
        Serial.println("Connected to WiFi");
        Serial.println("IP address: " +  String(wifi_if->get_ip_address()));
        Serial.println("MAC address: " + String(wifi_if->get_mac_address()));
    }
    else {
        Serial.println("Failed to connect to WiFi " + String(ret));
    }

    return ret == NSAPI_ERROR_OK ? WL_CONNECTED : WL_CONNECT_FAILED;
}

char* arduino::WiFiClass::SSID() {
    return _ssid;
}

int32_t arduino::WiFiClass::RSSI() {
    return wifi_if->get_rssi();
}

uint8_t arduino::WiFiClass::status() {
    // @todo: fix
    return WL_CONNECTED;
}

arduino::IPAddress arduino::WiFiClass::localIP() {
    arduino::IPAddress addr;

    const char *ip = wifi_if->get_ip_address();
    addr.fromString(ip); // @todo: the IP we get from Mbed is correct, but is parsed incorrectly by Arduino
    return addr;
}

NetworkInterface *arduino::WiFiClass::getNetwork() {
    return wifi_if;
}

// every specialization library should declare its own WiFI object: eg
//
// static ESP8266Interface wifi_if(PD_8, PD_9);
// arduino::WiFiClass WiFi(&wifi_if);
