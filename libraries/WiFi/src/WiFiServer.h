/*
  WiFiServer.h
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

#ifndef wifiserver_h
#define wifiserver_h

#include "WiFi.h"
#include "MbedServer.h"
#include "WiFiClient.h"

namespace arduino {

class WiFiClient;

class WiFiServer : public MbedServer {
  NetworkInterface* getNetwork() {
    return WiFi.getNetwork();
  }

public:
  WiFiServer() {}
  WiFiServer(uint16_t port)
    : MbedServer(port) {}
  WiFiClient accept(uint8_t* status = nullptr);
  WiFiClient available(uint8_t* status = nullptr) __attribute__((deprecated("Use accept().")));
};

}

#endif