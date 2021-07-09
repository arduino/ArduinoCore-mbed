/*
  MbedSSLClient.cpp - SSLClient implementation using mbed Sockets
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

#ifndef MBEDSSLCLIENT_H
#define MBEDSSLCLIENT_H

#include "MbedClient.h"

extern const char CA_CERTIFICATES[];

namespace arduino {

class MbedSSLClient : public arduino::MbedClient {

public:
  MbedSSLClient();
  virtual ~MbedSSLClient() {
    stop();
  }

  int connect(IPAddress ip, uint16_t port) {
    return connectSSL(ip, port);
  }
  int connect(const char* host, uint16_t port) {
    return connectSSL(host, port);
  }

private:
  int setRootCA() {
    return ((TLSSocket*)sock)->set_root_ca_cert_path("/wlan/");
  }
};

}

#endif /* MBEDSSLCLIENT_H */