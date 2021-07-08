/*
  MbedClient.h - Library for Arduino Wifi shield.
  Copyright (c) 2021 Arduino LLC.  All right reserved.

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

#ifndef MBEDCLIENT_H
#define MBEDCLIENT_H

#include "Arduino.h"
#include "SocketHelpers.h"
#include "api/Print.h"
#include "api/Client.h"
#include "api/IPAddress.h"
#include "TLSSocket.h"
#include "TCPSocket.h"
#include "rtos.h"

#ifndef SOCKET_BUFFER_SIZE
#define SOCKET_BUFFER_SIZE        256
#endif

namespace arduino {

class MbedClient : public arduino::Client {

public:
  MbedClient();
  MbedClient(MbedClient&& orig) {
    this->sock = orig.sock;
    orig.borrowed_socket = true;
    orig.stop();
    this->setSocket(orig.sock);
  }

  virtual ~MbedClient() {
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
    return sock != nullptr;
  }

  void setSocket(Socket* _sock);
  void configureSocket(Socket* _s);

  IPAddress remoteIP();
  uint16_t remotePort();

  friend class MbedServer;
  friend class MbedSSLClient;
  friend class MbedSocketClass;

  using Print::write;

protected:

  virtual NetworkInterface *getNetwork() = 0;
  Socket* sock = nullptr;

  void onBeforeConnect(mbed::Callback<int(void)> cb) {
    beforeConnect = cb;
  }

private:
  RingBufferN<SOCKET_BUFFER_SIZE> rxBuffer;
  bool _status = false;
  bool borrowed_socket = false;
  bool _own_socket = false;
  bool closing = false;
  mbed::Callback<int(void)> beforeConnect;
  SocketAddress address;
  rtos::Thread* reader_th = nullptr;
  rtos::EventFlags* event = nullptr;
  rtos::Mutex* mutex = nullptr;

  void readSocket();
  void getStatus();
};

}

#endif
