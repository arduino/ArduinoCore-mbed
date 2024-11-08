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

#include "WiFiSSLSE050Client.h"

arduino::MbedSSLSE050Client::MbedSSLSE050Client() {
  onBeforeConnect(mbed::callback(this, &MbedSSLSE050Client::setRootCAClientCertKey));
};

void arduino::MbedSSLSE050Client::setEccSlot(int KeySlot, const byte cert[], int certLen) {

  _keySlot = KeySlot;
  _certLen = certLen;
  _cert = cert;
}

void  WiFiSSLSE050Client::setEccSlot(int KeySlot, const byte cert[], int certLen) {
  if (!client) {
    newMbedClient();
  }
  static_cast<MbedSSLSE050Client*>(client.get())->setEccSlot(KeySlot, cert, certLen);
}

void WiFiSSLSE050Client::newMbedClient() {
  client.reset(new MbedSSLSE050Client());
  client->setNetwork(getNetwork());
}
