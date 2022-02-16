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

#include <AIoTC_Config.h>
#ifdef BOARD_HAS_SE050

#include "WiFiClient.h"

extern const char CA_CERTIFICATES[];

namespace arduino {

class WiFiSSLSE050Client : public arduino::WiFiClient {

public:
  WiFiSSLSE050Client();
  virtual ~WiFiSSLSE050Client() {
    stop();
  }

  int connect(IPAddress ip, uint16_t port) {
    return connectSSL(ip, port);
  }
  int connect(const char* host, uint16_t port) {
    return connectSSL(host, port);
  }

  void setEccSlot(int KeySlot, const char cert[], int certLen);

private:
  byte* _client_cert;
  int _client_cert_len;
  sss_object_t _keyObject;

  int setRootCA() {
    Serial.println("SET ROOT CA CERT");
    if( NSAPI_ERROR_OK != ((TLSSocket*)sock)->set_root_ca_cert_path("/wlan/")) {
      Serial.println("SET ROOT CA CERT ERROR");
    }

    Serial.println("SET CLIENT CERT");
    if( NSAPI_ERROR_OK != ((TLSSocket*)sock)->set_client_cert_key((void*)_client_cert, (size_t)_client_cert_len, &_keyObject)) {
      Serial.println("SET CLIENT CERT ERROR");
    }
  }

  int setClientCertKey() {
    return ((TLSSocket*)sock)->set_client_cert_key((void*)_client_cert, (size_t)_client_cert_len, &_keyObject);
  }
};

}

#endif

#endif /* WIFISSLSE050CLIENT_H */