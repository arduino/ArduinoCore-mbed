/*
  WiFiClient.cpp - Library for Arduino Wifi shield.
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

#ifndef wificlient_h
#define wificlient_h

#include "WiFi.h"
#include "api/Print.h"
#include "api/Client.h"
#include "api/IPAddress.h"
#include "TLSSocket.h"
#include "TCPSocket.h"

namespace arduino {

class WiFiClient : public arduino::Client {

public:
  WiFiClient();
  ~WiFiClient() {
    stop();
  }

  uint8_t status();
  int connect(SocketAddress socketAddress);
  int connect(IPAddress ip, uint16_t port);
  int connect(const char *host, uint16_t port);
  int connectSSL(SocketAddress socketAddress);
  int connectSSL(IPAddress ip, uint16_t port);
  int connectSSL(const char *host, uint16_t port);
  size_t write(uint8_t);
  size_t write(const uint8_t *buf, size_t size);
  int available();
  int read();
  int read(uint8_t *buf, size_t size);
  int peek();
  void flush();
  void stop();
  uint8_t connected();
  operator bool() {
    return sock != NULL;
  }

  void setSocket(Socket* _sock) {
    sock = _sock;
  }

  IPAddress remoteIP();
  uint16_t remotePort();

  friend class WiFiServer;
  friend class WiFiSSLClient;

  using Print::write;

protected:

  void onBeforeConnect(mbed::Callback<int(void)> cb) {
    beforeConnect = cb;
  }

private:
  static uint16_t _srcport;
  Socket* sock;
  RingBufferN<256> rxBuffer;
  uint8_t _status;
  mbed::Callback<int(void)> beforeConnect;

  void getStatus();
};

}

#endif