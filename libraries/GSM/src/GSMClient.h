/*
  GSMClient.h
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

#ifndef gsmclient_h
#define gsmclient_h

#include "GSM.h"
#include "AClient.h"

namespace arduino {

class GSMClient : public AClient {
private:
  NetworkInterface *getNetwork() {
    return GSM.getNetwork();
  }

public:
  size_t write(uint8_t b) {
    int ret = 0;
    do {
      ret = client->write(b);
      delay(0);
    } while (ret == 0 && status());
    return ret;
  }

  size_t write(const uint8_t *buf, size_t size) {
    int ret = 0;
    do {
      ret = client->write(buf, size);
      delay(0);
    } while (ret == 0 && status());
    return ret;
  }
};

}

#endif
