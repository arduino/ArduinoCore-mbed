/*
  AClient.h - Copyable Client implementation for Mbed Core

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

#ifndef MBEDACLIENT_H
#define MBEDACLIENT_H

#include <Arduino.h>
#include "MbedClient.h"

namespace arduino {

class AClient : public Client {
public:

  AClient() {}

  virtual int connect(IPAddress ip, uint16_t port);
  virtual int connect(const char *host, uint16_t port);
  int connectSSL(IPAddress ip, uint16_t port);
  int connectSSL(const char* host, uint16_t port);
  virtual void stop();

  virtual explicit operator bool();
  virtual uint8_t connected();
  uint8_t status();

  IPAddress remoteIP();
  uint16_t remotePort();

  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  virtual void flush();

  virtual int available();
  virtual int read();
  virtual int read(uint8_t *buf, size_t size);
  virtual int peek();

  using Print::write;

  void setSocketTimeout(unsigned long timeout);

protected:
  friend class EthernetServer;
  friend class WiFiServer;

  std::shared_ptr<MbedClient> client;
  virtual NetworkInterface* getNetwork() = 0;
  virtual void newMbedClient();
  void setSocket(Socket* sock);

};

class ASslClient : public AClient {
public:

  ASslClient() {}

  void disableSNI(bool statusSNI);

  void appendCustomCACert(const char* ca_cert);

protected:
  virtual void newMbedClient();
};

}
#endif
