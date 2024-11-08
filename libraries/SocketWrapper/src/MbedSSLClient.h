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

  virtual ~MbedSSLClient() {
    stop();
  }

  int connect(IPAddress ip, uint16_t port) {
    return connectSSL(ip, port);
  }
  int connect(const char* host, uint16_t port) {
    _hostname = host;
    return connectSSL(host, port);
  }
  void disableSNI(bool statusSNI) {
    _disableSNI = statusSNI;
  }

  void appendCustomCACert(const char* rootCA) {
    _rootCA = rootCA;
    _appendCA = true;
  }
  void setCACert(const char* rootCA) {
    _rootCA = rootCA;
    _appendCA = false;
  }
  void setCertificate(const char* clientCert) {
    _clientCert = clientCert;
  }
  void setPrivateKey(const char* privateKey) {
    _privateKey = privateKey;
  }

protected:
  const char* _rootCA;
  const char* _hostname;
  const char* _clientCert;
  const char* _privateKey;
  bool _disableSNI;
  bool _appendCA;

private:
  int setRootCA() {
    int err = 0;

    if(_hostname && !_disableSNI) {
      ((TLSSocket*)sock)->set_hostname(_hostname);
    }

    if(_clientCert && _privateKey) {
      err = ((TLSSocket*)sock)->set_client_cert_key(_clientCert, _privateKey);
      if( err != NSAPI_ERROR_OK) {
        return err;
      }
    }

    if(!_appendCA && _rootCA) {
      return ((TLSSocket*)sock)->set_root_ca_cert(_rootCA);
    }

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

    if(_rootCA != NULL) {
      err = ((TLSSocket*)sock)->append_root_ca_cert(_rootCA);
    }
    return err;
  }
};

}

#endif /* MBEDSSLCLIENT_H */
