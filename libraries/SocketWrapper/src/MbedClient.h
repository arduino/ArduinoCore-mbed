/*
  MbedClient.h - Client implementation using mbed Sockets
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
#define SOCKET_BUFFER_SIZE 256
#endif

namespace arduino {

class MbedClient {

public:
  MbedClient();

  MbedClient(unsigned long  timeout) {
    _timeout = timeout;
  }

  virtual ~MbedClient() {
    stop();
  }

  void setNetwork(NetworkInterface* network) {_network = network;}

  uint8_t status();
  int connect(SocketAddress socketAddress);
  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char* host, uint16_t port);
  int connectSSL(SocketAddress socketAddress);
  int connectSSL(IPAddress ip, uint16_t port);
  int connectSSL(const char* host, uint16_t port);
  size_t write(uint8_t);
  size_t write(const uint8_t* buf, size_t size);
  int available();
  int read();
  int read(uint8_t* buf, size_t size);
  int peek();
  void flush();
  void stop();
  uint8_t connected();
  operator bool() {
    return sock != nullptr;
  }

  void setSocket(Socket* _sock);
  Socket* getSocket() { return sock; };

  void configureSocket(Socket* _s);

  IPAddress remoteIP();
  uint16_t remotePort();

  void setSocketTimeout(unsigned long timeout);

  friend class MbedServer;
  friend class MbedSSLClient;
  friend class MbedSocketClass;

protected:
  NetworkInterface* getNetwork() {return _network;}

  NetworkInterface* _network = nullptr;
  Socket* sock = nullptr;

  void onBeforeConnect(mbed::Callback<int(void)> cb) {
    beforeConnect = cb;
  }

private:

  MbedClient(const MbedClient&) : _timeout(0) {}

  RingBufferN<SOCKET_BUFFER_SIZE> rxBuffer;
  bool _status = false;
  bool _own_socket = false;
  mbed::Callback<int(void)> beforeConnect;
  SocketAddress address;
  rtos::Thread* reader_th = nullptr;
  rtos::EventFlags* event = nullptr;
  rtos::Mutex* mutex = nullptr;
  unsigned long _timeout;

  void readSocket();
  void getStatus();
};

}

#endif
