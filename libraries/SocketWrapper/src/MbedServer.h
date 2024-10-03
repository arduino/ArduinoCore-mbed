/*
  MbedServer.h - Server implementation using mbed Sockets
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

class MbedServer {

protected:
  virtual NetworkInterface *getNetwork() = 0;
  TCPSocket *sock = nullptr;
  uint16_t _port;

public:
  MbedServer()
    : _port(80){};
  MbedServer(uint16_t port)
    : _port(port){};

  virtual ~MbedServer() {
    end();
  }
  void end() {
    if (sock) {
      delete sock;
      sock = nullptr;
    }
  }
  void begin(uint16_t port);
  void begin();
  uint8_t status();
  explicit operator bool() {
    return sock != nullptr;
  }

  friend class MbedSocketClass;
  friend class MbedClient;
};

}

#endif