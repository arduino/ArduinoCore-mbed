#include "WiFi.h"

bool arduino::WiFiClass::isVisible(char* ssid) {
    for (int i=0; i<10; i++) {
        if (strncmp(ap_list[i].get_ssid(), ssid, 32) == 0) {
            connected_ap = i;
            return true;
        }
    }
    return false;
}

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

    scanNetworks();
    // use scan result to populate security field
    if (!isVisible(ssid)) {
        return WL_CONNECT_FAILED;
    }

    nsapi_error_t ret = wifi_if->connect(ssid, passphrase, ap_list[connected_ap].get_security());

    return ret == NSAPI_ERROR_OK ? WL_CONNECTED : WL_CONNECT_FAILED;
}

char* arduino::WiFiClass::SSID() {
    return _ssid;
}

static const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

static uint8_t sec2enum(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return ENC_TYPE_NONE;
        case NSAPI_SECURITY_WEP:
            return ENC_TYPE_WEP;
        case NSAPI_SECURITY_WPA:
            return ENC_TYPE_TKIP;
        case NSAPI_SECURITY_WPA2:
            return ENC_TYPE_CCMP;
        case NSAPI_SECURITY_WPA_WPA2:
            return ENC_TYPE_CCMP;
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return ENC_TYPE_AUTO;
    }
}

int8_t arduino::WiFiClass::scanNetworks() {
    uint8_t count = 10;
    ap_list = new WiFiAccessPoint[count];
    return wifi_if->scan(ap_list, count);
}

char* arduino::WiFiClass::SSID(uint8_t networkItem) {
    return (char*)ap_list[networkItem].get_ssid();
}

int32_t arduino::WiFiClass::RSSI(uint8_t networkItem) {
    return ap_list[networkItem].get_rssi();
}

uint8_t arduino::WiFiClass::encryptionType(uint8_t networkItem) {
    return sec2enum(ap_list[networkItem].get_security());
}

int32_t arduino::WiFiClass::RSSI() {
    return wifi_if->get_rssi();
}

uint8_t arduino::WiFiClass::status() {
    // @todo: fix
    return WL_CONNECTED;
}

uint8_t arduino::WiFiClass::encryptionType() {
    return sec2enum(ap_list[connected_ap].get_security());
}

uint8_t* arduino::WiFiClass::BSSID(unsigned char* bssid) {
    const uint8_t* reverse_bssid = ap_list[connected_ap].get_bssid();
    for( int b = 0; b < 6; b++ ) {
        bssid[b] = reverse_bssid[5-b];
    }
    return bssid;
}

uint8_t* arduino::WiFiClass::macAddress(uint8_t* mac) {
    const char *mac_str = wifi_if->get_mac_address();
    for( int b = 0; b < 6; b++ )
    {
        uint32_t tmp;
        sscanf( &mac_str[b * 2 + (b)], "%02x", &tmp) ;
        mac[5-b] = (uint8_t)tmp ;
    }
    //sscanf(mac_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac[5], &mac[4], &mac[3], &mac[2], &mac[1], &mac[0]);
    return mac;
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

#if defined(ARDUINO_ENVIE_M7) || defined(ARDUINO_ENVIE_M4)
arduino::WiFiClass WiFi(WiFiInterface::get_default_instance());
#endif

// every specialization library should declare its own WiFI object: eg
//
// static ESP8266Interface wifi_if(PD_8, PD_9);
// arduino::WiFiClass WiFi(&wifi_if);
