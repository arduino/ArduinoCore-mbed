/*
  SocketHelpers.h - Utility wrappers for mbed Sockets and NetworkInterfaces
  Copyright (c) 2021 Arduino SA.  All right reserved.
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

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
  void config(const char* local_ip);

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

  /*
     * Get the DNS Server ip address.
     *
     * return: DNS Server ip address value
     */
  IPAddress dnsServerIP();

  virtual NetworkInterface* getNetwork() = 0;

  int download(char* url, const char* target, bool const is_https = false);

  int hostByName(const char* aHostname, IPAddress& aResult);

  uint8_t* macAddress(uint8_t* mac);

  void setFeedWatchdogFunc(voidFuncPtr func);
  void feedWatchdog();

  friend class MbedUDP;
  friend class MbedServer;
  friend class MbedClient;

protected:
  SocketAddress _ip = nullptr;
  SocketAddress _gateway = nullptr;
  SocketAddress _netmask = nullptr;
  SocketAddress _dnsServer1 = nullptr;
  SocketAddress _dnsServer2 = nullptr;

  voidFuncPtr _feed_watchdog_func = nullptr;

  FILE* download_target;

  void body_callback(const char* data, uint32_t data_len);

  static arduino::IPAddress ipAddressFromSocketAddress(SocketAddress socketAddress);
  static SocketAddress socketAddressFromIpAddress(arduino::IPAddress ip, uint16_t port);
};

using SocketHelpers = MbedSocketClass;

}

#endif
