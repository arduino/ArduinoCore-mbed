/*
  WiFiSSLSE050Client.h
  Copyright (c) 2022 Arduino SA.  All right reserved.

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

#ifndef WIFISSLSE050CLIENT_H
#define WIFISSLSE050CLIENT_H


#include "SE05X.h"
#include "WiFiSSLClient.h"

extern const char CA_CERTIFICATES[];

namespace arduino {

class WiFiSSLSE050Client : public arduino::WiFiSSLClient {

public:
  WiFiSSLSE050Client();
  virtual ~WiFiSSLSE050Client() {
    stop();
  }
  void setEccSlot(int KeySlot, const byte cert[], int certLen);

private:
  const byte* _client_cert;
  const char* _ca_cert;
  int _client_cert_len;
  int _keySlot;
  sss_object_t _keyObject;

  int setRootCAClientCertKey() {
    if( NSAPI_ERROR_OK != ((TLSSocket*)sock)->set_root_ca_cert_path("/wlan/")) {
      return 0;
    }

    if( NSAPI_ERROR_OK != ((TLSSocket*)sock)->append_root_ca_cert(_ca_cert_custom)) {
      return 0;
    }

    if(!SE05X.getObjectHandle(_keySlot, &_keyObject)) {
      return 0;
    }

    if( NSAPI_ERROR_OK != ((TLSSocket*)sock)->set_client_cert_key((void*)_client_cert, (size_t)_client_cert_len, &_keyObject, SE05X.getDeviceCtx())) {
      return 0;
    }

    return 1;
  }
};

}

#endif /* WIFISSLSE050CLIENT_H */
