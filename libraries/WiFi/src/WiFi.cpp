#include "WiFi.h"

bool arduino::WiFiClass::isVisible(const char* ssid) {
    for (int i=0; i<10; i++) {
        if (strncmp(ap_list[i].get_ssid(), ssid, 32) == 0) {
            connected_ap = i;
            return true;
        }
    }
    return false;
}

arduino::IPAddress arduino::WiFiClass::ipAddressFromSocketAddress(SocketAddress socketAddress) {
    nsapi_addr_t address = socketAddress.get_addr();
    return IPAddress(address.bytes[0], address.bytes[1], address.bytes[2], address.bytes[3]);    
}

int arduino::WiFiClass::begin(const char* ssid, const char *passphrase) {
    if (_ssid) free(_ssid);

    _ssid = (char*)malloc(33);
    if (!_ssid) {
        //tr_error("Could not allocate ssid buffer");
        return WL_CONNECT_FAILED;
    }

    if (wifi_if == nullptr) {
        //Q: What is the callback for?
        _initializerCallback();
        if(wifi_if == nullptr) return WL_CONNECT_FAILED;
    }

    memcpy(_ssid, ssid, 33);
    // too long? break it off
    if (strlen(ssid) > 32) _ssid[32] = 0;

    scanNetworks();
    // use scan result to populate security field
    if (!isVisible(_ssid)) {
        _currentNetworkStatus = WL_CONNECT_FAILED;
        return _currentNetworkStatus;
    }

    nsapi_error_t ret = wifi_if->connect(_ssid, passphrase, ap_list[connected_ap].get_security());

    _currentNetworkStatus = ret == NSAPI_ERROR_OK ? WL_CONNECTED : WL_CONNECT_FAILED;
    return _currentNetworkStatus;
}

int arduino::WiFiClass::beginAP(const char* ssid, const char *passphrase, uint8_t channel) {

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_PORTENTA_H7_M4)
    _softAP = WhdSoftAPInterface::get_default_instance();
#endif

    if (_softAP == NULL) {
        return WL_CONNECT_FAILED;
    }

    ensureDefaultAPNetworkConfiguration();

    //Set ap ssid, password and channel    
    static_cast<WhdSoftAPInterface*>(_softAP)->set_network(_ip, _netmask, _gateway);
    nsapi_error_t ret = static_cast<WhdSoftAPInterface*>(_softAP)->start(ssid, passphrase, NSAPI_SECURITY_WPA2, channel, true /* dhcp server */, NULL, true /* cohexistance */);

    return ret == NSAPI_ERROR_OK ? WL_AP_LISTENING : WL_CONNECT_FAILED;
}

void arduino::WiFiClass::ensureDefaultAPNetworkConfiguration() {
    if(_ip == nullptr){
        _ip = SocketAddress(DEFAULT_IP_ADDRESS);
    }
    if(_gateway == nullptr){
        _gateway = _ip;
    }
    if(_netmask == nullptr){
        _netmask = SocketAddress(DEFAULT_NETMASK);
    }
}

void arduino::WiFiClass::end() {
    disconnect();
}

int arduino::WiFiClass::disconnect() {
    if (_softAP != nullptr) {
        return static_cast<WhdSoftAPInterface*>(_softAP)->stop();        
    } else {
        return wifi_if->disconnect();
    }
}

void arduino::WiFiClass::config(arduino::IPAddress local_ip){    
    nsapi_addr_t convertedIP = {NSAPI_IPv4, {local_ip[0], local_ip[1], local_ip[2], local_ip[3]}};    
    _ip = SocketAddress(convertedIP);    
}

void arduino::WiFiClass::config(const char *local_ip){
    _ip = SocketAddress(local_ip);    
}

void arduino::WiFiClass::config(IPAddress local_ip, IPAddress dns_server){
    config(local_ip);
    setDNS(dns_server);    
}

void arduino::WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway){
    config(local_ip, dns_server);
    nsapi_addr_t convertedGatewayIP = {NSAPI_IPv4, {gateway[0], gateway[1], gateway[2], gateway[3]}};    
    _gateway = SocketAddress(convertedGatewayIP);
}

void arduino::WiFiClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet){    
    config(local_ip, dns_server, gateway);
    nsapi_addr_t convertedSubnetMask = {NSAPI_IPv4, {subnet[0], subnet[1], subnet[2], subnet[3]}};    
    _netmask = SocketAddress(convertedSubnetMask);
}

void arduino::WiFiClass::setDNS(IPAddress dns_server1){
    nsapi_addr_t convertedDNSServer = {NSAPI_IPv4, {dns_server1[0], dns_server1[1], dns_server1[2], dns_server1[3]}};    
    _dnsServer1 = SocketAddress(convertedDNSServer);
}

void arduino::WiFiClass::setDNS(IPAddress dns_server1, IPAddress dns_server2){
    setDNS(dns_server1);
    nsapi_addr_t convertedDNSServer2 = {NSAPI_IPv4, {dns_server2[0], dns_server2[1], dns_server2[2], dns_server2[3]}};    
    _dnsServer2 = SocketAddress(convertedDNSServer2);    
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
    if (ap_list == nullptr) {
        ap_list = new WiFiAccessPoint[count];
    }
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
    return _currentNetworkStatus;
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
    const char *mac_str = getNetwork()->get_mac_address();
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
    SocketAddress ip;
    NetworkInterface *interface = getNetwork();
    interface->get_ip_address(&ip);
    return ipAddressFromSocketAddress(ip);    
}

arduino::IPAddress arduino::WiFiClass::subnetMask() {    
    SocketAddress ip;
    NetworkInterface *interface = getNetwork();
    interface->get_netmask(&ip);
    return ipAddressFromSocketAddress(ip);    
}

arduino::IPAddress arduino::WiFiClass::gatewayIP() {    
    SocketAddress ip;
    NetworkInterface *interface = getNetwork();
    interface->get_gateway(&ip);
    return ipAddressFromSocketAddress(ip);    
}

NetworkInterface *arduino::WiFiClass::getNetwork() {
    if (_softAP != nullptr) {
        return _softAP;
    } else {
        return wifi_if;
    }
}

unsigned long arduino::WiFiClass::getTime() {
    return 0;
}

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_PORTENTA_H7_M4)

#include "whd_version.h"
char* arduino::WiFiClass::firmwareVersion() {
    return WHD_VERSION;
}

arduino::WiFiClass WiFi(WiFiInterface::get_default_instance());
#endif

// every specialization library should declare its own WiFI object: eg
//
// static ESP8266Interface wifi_if(PD_8, PD_9);
// arduino::WiFiClass WiFi(&wifi_if);
