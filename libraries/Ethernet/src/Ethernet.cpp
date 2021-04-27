#include "Ethernet.h"

#define SSID_MAX_LENGTH 32

arduino::IPAddress arduino::EthernetClass::ipAddressFromSocketAddress(SocketAddress socketAddress) {
    nsapi_addr_t address = socketAddress.get_addr();
    return IPAddress(address.bytes[0], address.bytes[1], address.bytes[2], address.bytes[3]);    
}

SocketAddress arduino::EthernetClass::socketAddressFromIpAddress(arduino::IPAddress ip, uint16_t port) {
    nsapi_addr_t convertedIP = {NSAPI_IPv4, {ip[0], ip[1], ip[2], ip[3]}};    
    return SocketAddress(convertedIP, port);
}

int arduino::EthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout) {
    if (eth_if == nullptr) {
        //Q: What is the callback for?
        _initializerCallback();
        if(eth_if == nullptr) return 0;
    }

    unsigned long start = millis();
    eth_if->set_blocking(false);
    nsapi_error_t result = eth_if->connect();

    while ((millis() - start < timeout) && (linkStatus() != LinkON)) {
    	delay(10);
    }

    return (linkStatus() == LinkON ? 1 : 0);
}

void arduino::EthernetClass::end() {
    disconnect();
}

EthernetLinkStatus arduino::EthernetClass::linkStatus() {
	return (eth_if->get_connection_status() == NSAPI_STATUS_GLOBAL_UP ? LinkON : LinkOFF);
}

EthernetHardwareStatus arduino::EthernetClass::hardwareStatus() {
	return EthernetMbed;
}


int arduino::EthernetClass::disconnect() {
    eth_if->disconnect();
}

void arduino::EthernetClass::config(arduino::IPAddress local_ip){    
    nsapi_addr_t convertedIP = {NSAPI_IPv4, {local_ip[0], local_ip[1], local_ip[2], local_ip[3]}};    
    _ip = SocketAddress(convertedIP);    
}

void arduino::EthernetClass::config(const char *local_ip){
    _ip = SocketAddress(local_ip);    
}

void arduino::EthernetClass::config(IPAddress local_ip, IPAddress dns_server){
    config(local_ip);
    setDNS(dns_server);    
}

void arduino::EthernetClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway){
    config(local_ip, dns_server);
    nsapi_addr_t convertedGatewayIP = {NSAPI_IPv4, {gateway[0], gateway[1], gateway[2], gateway[3]}};    
    _gateway = SocketAddress(convertedGatewayIP);
}

void arduino::EthernetClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet){    
    config(local_ip, dns_server, gateway);
    nsapi_addr_t convertedSubnetMask = {NSAPI_IPv4, {subnet[0], subnet[1], subnet[2], subnet[3]}};    
    _netmask = SocketAddress(convertedSubnetMask);
}

void arduino::EthernetClass::setDNS(IPAddress dns_server1){
    nsapi_addr_t convertedDNSServer = {NSAPI_IPv4, {dns_server1[0], dns_server1[1], dns_server1[2], dns_server1[3]}};    
    _dnsServer1 = SocketAddress(convertedDNSServer);
}

void arduino::EthernetClass::setDNS(IPAddress dns_server1, IPAddress dns_server2){
    setDNS(dns_server1);
    nsapi_addr_t convertedDNSServer2 = {NSAPI_IPv4, {dns_server2[0], dns_server2[1], dns_server2[2], dns_server2[3]}};    
    _dnsServer2 = SocketAddress(convertedDNSServer2);    
}

uint8_t arduino::EthernetClass::status() {    
    return _currentNetworkStatus;
}

int arduino::EthernetClass::hostByName(const char* aHostname, IPAddress& aResult){
    SocketAddress socketAddress = SocketAddress();
    nsapi_error_t returnCode = getNetwork()->gethostbyname(aHostname, &socketAddress);
    nsapi_addr_t address = socketAddress.get_addr();
    aResult[0] = address.bytes[0];
    aResult[1] = address.bytes[1];
    aResult[2] = address.bytes[2];
    aResult[3] = address.bytes[3];    
    return returnCode == NSAPI_ERROR_OK ? 1 : 0;
}

uint8_t* arduino::EthernetClass::macAddress(uint8_t* mac) {
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

arduino::IPAddress arduino::EthernetClass::localIP() {    
    SocketAddress ip;
    NetworkInterface *interface = getNetwork();
    interface->get_ip_address(&ip);
    return ipAddressFromSocketAddress(ip);    
}

arduino::IPAddress arduino::EthernetClass::subnetMask() {    
    SocketAddress ip;
    NetworkInterface *interface = getNetwork();
    interface->get_netmask(&ip);
    return ipAddressFromSocketAddress(ip);    
}

arduino::IPAddress arduino::EthernetClass::gatewayIP() {    
    SocketAddress ip;
    NetworkInterface *interface = getNetwork();
    interface->get_gateway(&ip);
    return ipAddressFromSocketAddress(ip);    
}

NetworkInterface *arduino::EthernetClass::getNetwork() {
    return eth_if;
}

unsigned long arduino::EthernetClass::getTime() {
    return 0;
}

arduino::EthernetClass Ethernet;
