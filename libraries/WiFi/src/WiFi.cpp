#include "WiFi.h"

int arduino::WiFiClass::begin(char* ssid, const char *passphrase) {
    if (_ssid) free(_ssid);

    _ssid = (char*)malloc(33);
    if (!_ssid) {
        //tr_error("Could not allocate ssid buffer");
        return WL_CONNECT_FAILED;
    }

    // too long? break it off
    if (strlen(ssid) > 32) ssid[32] = 0;
    memcpy(_ssid, ssid, 33);

    //tr_debug("Connecting to '%s'", _ssid);
    nsapi_error_t ret = wifi_if->connect(ssid, passphrase, NSAPI_SECURITY_WPA2);

    if (ret == NSAPI_ERROR_OK) {
        //tr_debug("Connected to WiFi");
        //tr_debug("IP address: %s", wifi_if->get_ip_address());
        //tr_debug("MAC address: %s", wifi_if->get_mac_address());
    }
    else {
        //tr_debug("Failed to connect to WiFi (%d)", ret);
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

NetworkInterface *arduino::WiFiClass::get_network() {
    return wifi_if;
}

// every specialization library should declare its own WiFI object: eg
// 
// WiFiClass WiFi(&);
