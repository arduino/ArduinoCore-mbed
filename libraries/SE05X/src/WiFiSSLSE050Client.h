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
#include "MbedSSLClient.h"

extern const char CA_CERTIFICATES[];

namespace arduino {

class MbedSSLSE050Client : public arduino::MbedSSLClient {

public:
  MbedSSLSE050Client();

  void setEccSlot(int KeySlot, const byte cert[], int certLen);

private:
  const byte* _cert;
  int _certLen;
  int _keySlot;
  sss_object_t _keyObject;

  int setRootCAClientCertKey() {
    int err = setRootCA();
    if (err != NSAPI_ERROR_OK) {
      return err;
    }

    if(SE05X.getObjectHandle(_keySlot, &_keyObject) != NSAPI_ERROR_OK) {
      return NSAPI_ERROR_DEVICE_ERROR;
    }

    if(((TLSSocket*)sock)->set_client_cert_key((void*)_cert,
                                               (size_t)_certLen,
                                               &_keyObject,
                                               SE05X.getDeviceCtx()) != NSAPI_ERROR_OK) {
      return NSAPI_ERROR_DEVICE_ERROR;
    }
    return NSAPI_ERROR_OK;
  }
};

class WiFiSSLSE050Client : public arduino::WiFiSSLClient {

public:

  void setEccSlot(int KeySlot, const byte cert[], int certLen);

protected:
  virtual void newMbedClient();
};

}

#endif /* WIFISSLSE050CLIENT_H */
