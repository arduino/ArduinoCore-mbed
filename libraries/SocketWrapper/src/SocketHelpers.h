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
   * param local_ip: Static ip configuration as string
   */
  void config(const char* local_ip);

  /* Change Ip configuration settings disabling the dhcp client
   *
   * param local_ip:   Static ip configuration
	* param dns_server: IP configuration for DNS server 1
   */
  void config(IPAddress local_ip, IPAddress dns_server);

  /* Change Ip configuration settings disabling the dhcp client
   *
   * param local_ip:    Static ip configuration
	* param dns_server:  IP configuration for DNS server 1
   * param gateway :    Static gateway configuration
   */
  void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway);

  /* Change Ip configuration settings disabling the dhcp client
   *
   * param local_ip:   Static ip configuration
	* param dns_server: IP configuration for DNS server 1
   * param gateway:    Static gateway configuration
   * param subnet:     Static Subnet mask
   */
  void config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet);

  // When using DHCP the hostname provided will be used.
  int setHostname(const char* hostname);

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

  /*
   * Get the DNS Server ip address.
   *
   * return: DNS Server ip address value
   */
  IPAddress dnsIP(int n = 0);

  virtual NetworkInterface* getNetwork() = 0;

  /*
   * Ping the specified target.
   *
   * ttl value is unused, but kept for API compatibility
   *
   * return: RTT in milliseconds or -1 on error
   */
  int ping(const char* hostname, uint8_t ttl = 255, uint32_t timeout = 5000);
  int ping(const String &hostname, uint8_t ttl = 255, uint32_t timeout = 5000);
  int ping(IPAddress host, uint8_t ttl = 255, uint32_t timeout = 5000);

  /*
   * Download a file from an HTTP endpoint and save it in the provided `target` location on the fs
   * The parameter cbk can be used to perform actions on the buffer before saving on the fs
   *
   * return: on success the size of the downloaded file, on error -status code
   */
  int download(
    const char* url, const char* target, bool const is_https = false);
  /*
   * Download a file from an HTTP endpoint and handle the body of the request on a callback
   * passed as an argument
   *
   * return: on success the size of the downloaded file, on error -status code
   */
  int download(
    const char* url, bool const is_https = false,
    mbed::Callback<void(const char*, uint32_t)> cbk = nullptr);

  int hostByName(const char* aHostname, IPAddress& aResult);

  /*
   * Get the interface MAC address.
   *
   * Network interface should be ready to get a valid mac address.
   * Call WiFi.begin("",""); or Ethernet.begin(); before issuing a mac address
   * request, otherwhise returned value will be ff:ff:ff:ff:ff:ff
   *
   * return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
   */
  uint8_t* macAddress(uint8_t* mac);

  /*
   * Get the interface MAC address String.
   *
   * Network interface should be ready to get a valid MAC address.
   * Call WiFi.begin("",""); or Ethernet.begin(); before issuing a mac address
   * request, otherwhise returned value will be ff:ff:ff:ff:ff:ff
   *
   * return: MAC Address String
   */
  String macAddress();

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
  bool _useStaticIP = false;

  voidFuncPtr _feed_watchdog_func = nullptr;

  FILE* download_target;

  void body_callback(const char* data, uint32_t data_len);

  int ping(SocketAddress &socketAddress, uint8_t ttl, uint32_t timeout);
  static arduino::IPAddress ipAddressFromSocketAddress(SocketAddress socketAddress);
  static SocketAddress socketAddressFromIpAddress(arduino::IPAddress ip, uint16_t port);
  static nsapi_error_t gethostbyname(NetworkInterface* interface, const char* aHostname, SocketAddress* socketAddress);
};

using SocketHelpers = MbedSocketClass;

}

#endif
