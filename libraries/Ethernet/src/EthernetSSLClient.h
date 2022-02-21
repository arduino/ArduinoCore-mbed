/*
  EthernetSSLClient.h
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

#ifndef ETHERNETSSLCLIENT_H
#define ETHERNETSSLCLIENT_H

#include "EthernetClient.h"

#include <FATFileSystem.h>
#include <MBRBlockDevice.h>
#include <QSPIFBlockDevice.h>

extern const char CA_CERTIFICATES[];

namespace arduino {

class EthernetSSLClient : public arduino::EthernetClient {

public:
  EthernetSSLClient();
  virtual ~EthernetSSLClient() {
    stop();
  }

  int connect(IPAddress ip, uint16_t port) {
    return connectSSL(ip, port);
  }
  int connect(const char* host, uint16_t port) {
    return connectSSL(host, port, _disableSNI);
  }
  void disableSNI(bool statusSNI) {
    _disableSNI = statusSNI;
  }

private:
  int setRootCA() {

    QSPIFBlockDevice root;
    mbed::MBRBlockDevice wifi_data(&root, 1);
    mbed::FATFileSystem wifi("wlan");

    int err = wifi.mount(&wifi_data);
    if (err)
      return err;

    return ((TLSSocket*)sock)->set_root_ca_cert_path("/wlan/");
  }

  bool _disableSNI;
};

}

#endif /* EthernetSSLCLIENT_H */
