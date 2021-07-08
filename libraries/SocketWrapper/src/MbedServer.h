/*
  WiFiServer.h - Library for Arduino Wifi shield.
  Copyright (c) 2011-2014 Arduino LLC.  All right reserved.
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

#ifndef MBEDSERVER_H
#define MBEDSERVER_H

#include "Arduino.h"
#include "SocketHelpers.h"
#include "mbed.h"
#include "api/Print.h"
#include "api/Client.h"
#include "api/IPAddress.h"
#include "TLSSocket.h"
#include "TCPSocket.h"

namespace arduino {

class MbedClient;

class MbedServer : public arduino::Server {
private:
  uint16_t _port;
  TCPSocket* sock = nullptr;

protected:
  virtual NetworkInterface *getNetwork() = 0;

public:
  MbedServer(uint16_t);
  virtual ~MbedServer() {
    if (sock) {
      delete sock;
      sock = nullptr;
    }
  }
  void begin();
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  uint8_t status();

  using Print::write;

  friend class MbedSocketClass;
  friend class MbedClient;
};

}

#endif