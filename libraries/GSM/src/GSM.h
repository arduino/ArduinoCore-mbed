/*
  GSM.h - Library for GSM on mbed platforms.
  Copyright (c) 2011-2021 Arduino LLC.  All right reserved.
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

#ifndef GSM_h
#define GSM_h

#include <inttypes.h>

extern "C" {
#include "utility/wl_definitions.h"
#include "utility/wl_types.h"
}

#include "SocketHelpers.h"
#include "CellularContext.h"

#include "GEMALTO_CINTERION.h"
#include "GENERIC_AT3GPP.h"

#include "drivers/BufferedSerial.h"

#define MBED_CONF_GEMALTO_CINTERION_TX    PA_0
#define MBED_CONF_GEMALTO_CINTERION_RX    PI_9
#define MBED_CONF_GEMALTO_CINTERION_RTS   PI_10
//#define MBED_CONF_GEMALTO_CINTERION_CTS   PI_13
#define MBED_CONF_APP_SOCK_TYPE           1

namespace arduino {

typedef void* (*voidPrtFuncPtr)(void);

class GSMClass : public MbedSocketClass {
public:

  GSMClass(NetworkInterface* _if)
    : _rat(CATNB),
      gsm_if(_if),
      _context((mbed::CellularContext*)gsm_if),
      _device(((mbed::CellularContext*)gsm_if)->get_device()) {}

  /* Start GSM connection.
     * Configure the credentials into the device.
     *
     * param pin: Pointer to the pin string.
     * param apn: Pointer to the apn string.
     * param username: Pointer to the username string.
     * param password: Pointer to the password string.
     * param rat: Radio Access Technology.
     * 
     * return: 0 in case of success, negative number in case of failure
     */
  int begin(const char* pin, const char* apn, const char* username, const char* password, RadioAccessTechnologyType rat = CATNB);

  /*
     * Disconnect from the network
     *
     * return: one value of wl_status_t enum
     */
  int disconnect(void);

  void end(void);

  unsigned long getTime();

  unsigned long getLocalTime();

  bool setTime(unsigned long const epoch, int const timezone = 0);

  void debug(void);

  int ping(const char* hostname, uint8_t ttl = 128);
  int ping(const String& hostname, uint8_t ttl = 128);
  int ping(IPAddress host, uint8_t ttl = 128);

  friend class GSMClient;
  friend class GSMUDP;

  NetworkInterface* getNetwork();

private:
  const char* _pin = nullptr;
  const char* _apn = nullptr;
  const char* _username = nullptr;
  const char* _password = nullptr;
  RadioAccessTechnologyType _rat;
  NetworkInterface* gsm_if = nullptr;
  mbed::CellularContext* _context = nullptr;
  mbed::CellularDevice* _device = nullptr;
};

}

extern GSMClass GSM;

#include "GSMClient.h"
#include "GSMUdp.h"

#endif
