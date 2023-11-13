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
#include <FATFileSystem.h>
#include <MBRBlockDevice.h>

extern const char CA_CERTIFICATES[];

namespace arduino {

class MbedSSLClient : public arduino::MbedClient {

public:
  MbedSSLClient();

  MbedSSLClient(unsigned long  timeout);

  virtual ~MbedSSLClient() {
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

  void appendCustomCACert(const char* ca_cert) {
    _ca_cert_custom = ca_cert;
  }

protected:
  const char* _ca_cert_custom = NULL;

private:
  int setRootCA() {
    int err = 0;

#if defined(MBEDTLS_FS_IO)
    mbed::BlockDevice* root = mbed::BlockDevice::get_default_instance();
    err = root->init();
    if( err != 0) {
      return err;
    }

    mbed::MBRBlockDevice wifi_data(root, 1);
    mbed::FATFileSystem wifi("wlan");

    err = wifi.mount(&wifi_data);
    if (err) {
      return err;
    }

    err = ((TLSSocket*)sock)->set_root_ca_cert_path("/wlan/");
    if( err != NSAPI_ERROR_OK) {
      return err;
    }
#endif

    if(_ca_cert_custom != NULL) {
      err = ((TLSSocket*)sock)->append_root_ca_cert(_ca_cert_custom);
    }
    return err;
  }

  bool _disableSNI;
};

}

#endif /* MBEDSSLCLIENT_H */
