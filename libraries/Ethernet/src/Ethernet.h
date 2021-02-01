/* Copyright 2018 Paul Stoffregen
 * Copyright 2020 Arduino SA
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ethernet_h_
#define ethernet_h_

#include "Arduino.h"
#include "api/IPAddress.h"
#include "EthernetClient.h"
#include "EthernetServer.h"
#include "EthernetUdp.h"

#include "netsocket/NetworkInterface.h"
#include "EthernetInterface.h"

enum EthernetLinkStatus {
	Unknown,
	LinkON,
	LinkOFF
};

enum EthernetHardwareStatus {
	EthernetNoHardware,
	EthernetMbed = 6
};

namespace arduino {

typedef void* (*voidPrtFuncPtr)(void);

class EthernetClass {

public:
	// Initialise the Ethernet shield to use the provided MAC address and
	// gain the rest of the configuration through DHCP.
	// Returns 0 if the DHCP configuration failed, and 1 if it succeeded
    EthernetClass(EthernetInterface* _if) : eth_if(_if) {};
    EthernetClass() {};

    EthernetClass(voidPrtFuncPtr _cb) : _initializerCallback(_cb) {};

    int begin(uint8_t *mac = nullptr , unsigned long timeout = 60000, unsigned long responseTimeout = 4000);
    // int begin(unsigned long timeout = 60000, unsigned long responseTimeout = 4000) { return begin(nullptr, timeout, responseTimeout); };
	int maintain();
	EthernetLinkStatus linkStatus();
	EthernetHardwareStatus hardwareStatus();

	// Manaul configuration
	void begin(uint8_t *mac, IPAddress ip) {}
	void begin(uint8_t *mac, IPAddress ip, IPAddress dns) {}
	void begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway) {}
	void begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet) {}
	void begin(IPAddress ip) {}
	void begin(IPAddress ip, IPAddress dns) {}
	void begin(IPAddress ip, IPAddress dns, IPAddress gateway) {}
	void begin(IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet) {}
	void init(uint8_t sspin = 10);

	void MACAddress(uint8_t *mac_address);
    uint8_t* macAddress(uint8_t* mac);
	IPAddress localIP();
	IPAddress subnetMask();
	IPAddress gatewayIP();
	IPAddress dnsServerIP() { return ipAddressFromSocketAddress(_dnsServer1); }

    void config(IPAddress local_ip);
    void config(const char *local_ip);
    void config(IPAddress local_ip, IPAddress dns_server);
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway);
    void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);
    void setDNS(IPAddress dns_server1);
    void setDNS(IPAddress dns_server1, IPAddress dns_server2);
    void setHostname(const char* name);

    int disconnect(void);
    void end(void);

    uint8_t status();
    int hostByName(const char* aHostname, IPAddress& aResult);
    unsigned long getTime();

	void setMACAddress(const uint8_t *mac_address);
	void setLocalIP(const IPAddress local_ip);
	void setSubnetMask(const IPAddress subnet);
	void setGatewayIP(const IPAddress gateway);
	void setDnsServerIP(const IPAddress dns_server) { _dnsServer1 = socketAddressFromIpAddress(dns_server, 0); }
	void setRetransmissionTimeout(uint16_t milliseconds);
	void setRetransmissionCount(uint8_t num);

    friend class EthernetClient;
    friend class EthernetServer;
    friend class EthernetUDP;

    NetworkInterface *getNetwork();

private:

	volatile EthernetLinkStatus _currentNetworkStatus = Unknown;
	EthernetInterface net;
    SocketAddress _ip = nullptr;
    SocketAddress _gateway = nullptr;
    SocketAddress _netmask = nullptr;
    SocketAddress _dnsServer1 = nullptr;
    SocketAddress _dnsServer2 = nullptr;
    EthernetInterface* eth_if = &net;
    voidPrtFuncPtr _initializerCallback;
    arduino::IPAddress ipAddressFromSocketAddress(SocketAddress socketAddress);
    SocketAddress socketAddressFromIpAddress(arduino::IPAddress ip, uint16_t port);
};

}

extern arduino::EthernetClass Ethernet;

#endif
