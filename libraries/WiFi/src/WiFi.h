/*
  WiFi.h - Library for Wifi on mbed platforms.
  Copyright (c) 2011-2021 Arduino LLC.  All right reserved.
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

#ifndef WiFi_h
#define WiFi_h

#include <inttypes.h>

extern "C" {
#include "utility/wl_definitions.h"
#include "utility/wl_types.h"
}

#include "SocketHelpers.h"

#if defined(COMPONENT_4343W_FS)
#include "WhdSoftAPInterface.h"
#endif

#ifndef DEFAULT_IP_ADDRESS
#define DEFAULT_IP_ADDRESS "192.168.3.1"
#endif

#ifndef DEFAULT_NETMASK
#define DEFAULT_NETMASK "255.255.255.0"
#endif

#ifndef DEFAULT_AP_CHANNEL
#define DEFAULT_AP_CHANNEL 6
#endif

#define ARDUINO_PORTENTA_H7_WIFI_HAS_FEED_WATCHDOG_FUNC

namespace arduino {

class WiFiClass : public MbedSocketClass {
public:
  static int16_t _state[MAX_SOCK_NUM];
  static uint16_t _server_port[MAX_SOCK_NUM];

  WiFiClass(WiFiInterface* _if)
    : wifi_if(_if){};

  /*
   * Get firmware version
   */
  static const char* firmwareVersion();

  /* Start Wifi connection for OPEN networks
   *
   * param ssid: Pointer to the SSID string.
   */
  int begin(const char* ssid);

  void MACAddress(uint8_t *mac_address) __attribute__((deprecated("Use macAddress(uint8_t *mac_address)")));

  /* Start Wifi connection with passphrase
   * the most secure supported mode will be automatically selected
   *
   * param ssid: Pointer to the SSID string.
   * param passphrase: Passphrase. Valid characters in a passphrase
   *        must be between ASCII 32-126 (decimal).
   */
  int begin(const char* ssid, const char* passphrase, wl_enc_type security = ENC_TYPE_UNKNOWN);

  // Inherit config methods from the parent class
  using MbedSocketClass::config;

  void config(const char* localip, const char* netmask, const char* gateway);

  int beginAP(const char* ssid, const char* passphrase, uint8_t channel = DEFAULT_AP_CHANNEL);

  /*
   * Disconnect from the network
   *
   * return: one value of wl_status_t enum
   */
  int disconnect(void);

  void end(void);

  /*
   * Return the current SSID associated with the network
   *
   * return: ssid string
   */
  char* SSID();

  /*
   * Return the current BSSID associated with the network.
   * It is the MAC address of the Access Point
   *
   * return: pointer to uint8_t array with length WL_MAC_ADDR_LENGTH
   */
  uint8_t* BSSID(uint8_t* bssid);

  /*
   * Return the current RSSI /Received Signal Strength in dBm)
   * associated with the network
   *
   * return: signed value
   */
  int32_t RSSI();

  /*
   * Return the Encryption Type associated with the network
   *
   * return: one value of wl_enc_type enum
   */
  uint8_t encryptionType();

  /*
   * Start scan WiFi networks available
   *
   * return: Number of discovered networks
   */
  int8_t scanNetworks();

  /*
   * Return the SSID discovered during the network scan.
   *
   * param networkItem: specify from which network item want to get the information
   *
   * return: ssid string of the specified item on the networks scanned list
   */
  char* SSID(uint8_t networkItem);

  /*
   * Return the encryption type of the networks discovered during the scanNetworks
   *
   * param networkItem: specify from which network item want to get the information
   *
   * return: encryption type (enum wl_enc_type) of the specified item on the networks scanned list
   */
  uint8_t encryptionType(uint8_t networkItem);

  uint8_t* BSSID(uint8_t networkItem, uint8_t* bssid);
  uint8_t channel(uint8_t networkItem);

  /*
   * Return the RSSI of the networks discovered during the scanNetworks
   *
   * param networkItem: specify from which network item want to get the information
   *
   * return: signed value of RSSI of the specified item on the networks scanned list
   */
  int32_t RSSI(uint8_t networkItem);

  /*
   * Return Connection status.
   *
   * return: one of the value defined in wl_status_t
   */
  uint8_t status();

  unsigned long getTime();

  /*
   * Configure WiFi join timeout in milliseconds. Default value is 7s.
   */
  void setTimeout(unsigned long timeout);

  friend class WiFiClient;
  friend class WiFiServer;
  friend class WiFiUDP;
  friend class WiFiSSLClient;

  NetworkInterface* getNetwork();

private:
  EMACInterface* _softAP = nullptr;
  char* _ssid = nullptr;
  volatile wl_status_t _currentNetworkStatus = WL_IDLE_STATUS;
  WiFiInterface* wifi_if = nullptr;
  WiFiAccessPoint* ap_list = nullptr;
  uint8_t connected_ap;
  nsapi_security_t _security;
  unsigned long _timeout = 7000;
  int setSSID(const char* ssid);
  void ensureDefaultAPNetworkConfiguration();
  static void* handleAPEvents(whd_interface_t ifp, const whd_event_header_t* event_header, const uint8_t* event_data, void* handler_user_data);
  bool isVisible(const char* ssid);
  static void statusCallback(nsapi_event_t status, intptr_t param);
};

}

extern WiFiClass WiFi;

#include "WiFiClient.h"
#include "WiFiServer.h"
#include "WiFiUdp.h"
#include "WiFiSSLClient.h"

#endif
