#ifndef MBEDSOCKETCLASS_H
#define MBEDSOCKETCLASS_H

#include "Arduino.h"
#include "netsocket/NetworkInterface.h"

namespace arduino {

class MbedSocketClass {

public:
    void config(IPAddress local_ip);
    
    /* Change Ip configuration settings disabling the dhcp client
        *
        * param local_ip: 	Static ip configuration as string
        */
    void config(const char *local_ip);

    /* Change Ip configuration settings disabling the dhcp client
        *
        * param local_ip: 	Static ip configuration
	* param dns_server:     IP configuration for DNS server 1
        */
    void config(IPAddress local_ip, IPAddress dns_server);

    /* Change Ip configuration settings disabling the dhcp client
        *
        * param local_ip: 	Static ip configuration
	* param dns_server:     IP configuration for DNS server 1
        * param gateway : 	Static gateway configuration
        */
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway);

    /* Change Ip configuration settings disabling the dhcp client
        *
        * param local_ip: 	Static ip configuration
	* param dns_server:     IP configuration for DNS server 1
        * param gateway: 	Static gateway configuration
        * param subnet:		Static Subnet mask
        */
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);

    /* Change DNS Ip configuration
     *
     * param dns_server1: ip configuration for DNS server 1
     */
    void setDNS(IPAddress dns_server1);

    /* Change DNS Ip configuration
     *
     * param dns_server1: ip configuration for DNS server 1
     * param dns_server2: ip configuration for DNS server 2
     *
     */
    void setDNS(IPAddress dns_server1, IPAddress dns_server2);

    /*
     * Get the interface IP address.
     *
     * return: Ip address value
     */
    IPAddress localIP();

    /*
     * Get the interface subnet mask address.
     *
     * return: subnet mask address value
     */
    IPAddress subnetMask();

    /*
     * Get the gateway ip address.
     *
     * return: gateway ip address value
     */
    IPAddress gatewayIP();

    virtual NetworkInterface *getNetwork() = 0;

    int download(char* url, const char* target, bool const is_https = false);

    int hostByName(const char* aHostname, IPAddress& aResult);

    uint8_t* macAddress(uint8_t* mac);

    friend class MbedUDP;
    friend class MbedServer;
    friend class MbedClient;

protected:
	SocketAddress _ip = nullptr;
    SocketAddress _gateway = nullptr;
    SocketAddress _netmask = nullptr;
    SocketAddress _dnsServer1 = nullptr;
    SocketAddress _dnsServer2 = nullptr;

    static arduino::IPAddress ipAddressFromSocketAddress(SocketAddress socketAddress);
    static SocketAddress socketAddressFromIpAddress(arduino::IPAddress ip, uint16_t port);
};

using SocketHelpers = MbedSocketClass;

}

#endif