/*
  GSM.h - Library for GSM on mbed platforms.
  Copyright (c) 2011-2023 Arduino LLC.  All right reserved.

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

#include "SocketHelpers.h"
#include "CellularContext.h"

#include "GEMALTO_CINTERION.h"
#include "GENERIC_AT3GPP.h"

#include "drivers/BufferedSerial.h"
#include "drivers/UnbufferedSerial.h"
#include "CMUXClass.h"
#include "PTYSerial.h"

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_PORTENTA_H7_M4)
  #define MBED_CONF_GEMALTO_CINTERION_TX    PA_0
  #define MBED_CONF_GEMALTO_CINTERION_RX    PI_9
  #define MBED_CONF_GEMALTO_CINTERION_RTS   PI_10
  #define MBED_CONF_GEMALTO_CINTERION_CTS   PI_13
  #define MBED_CONF_GEMALTO_CINTERION_RST   PJ_10
  #define MBED_CONF_GEMALTO_CINTERION_ON    PJ_7
#elif defined (ARDUINO_EDGE_CONTROL)
  /* IMPORTANT: turn on the module's 5V on demand by calling
     pinMode(ON_MKR2, OUTPUT);
     digitalWrite(ON_MKR2, HIGH);
  */
  #define MBED_CONF_GEMALTO_CINTERION_TX    p24
  #define MBED_CONF_GEMALTO_CINTERION_RX    p25
  #define MBED_CONF_GEMALTO_CINTERION_RTS   NC
  #define MBED_CONF_GEMALTO_CINTERION_CTS   NC
  #define MBED_CONF_GEMALTO_CINTERION_RST   p31
  #define MBED_CONF_GEMALTO_CINTERION_ON    p2
#else
  #error Gemalto Cinterion cellular connectivity not supported
#endif

#if defined __has_include
  #if __has_include ("GPS.h")
  #  define _CMUX_ENABLE 1
  #else
  #   define _CMUX_ENABLE 0
  #endif
#endif

namespace arduino {

class GSMClass : public MbedSocketClass {
public:

  GSMClass()
    : _rat(CATNB) {
      if(_CMUX_ENABLE){
        arduino::GSMClass::enableCmux();
      }
    }

  /*
   * Start GSM connection. Configure the credentials into the device.
   *
   * param pin: Pointer to the pin string.
   * param apn: Pointer to the apn string.
   * param username: Pointer to the username string.
   * param password: Pointer to the password string.
   * param rat: Radio Access Technology.
   *
   * return: 0 in case of success, negative number in case of failure
   */
  int begin(const char* pin, const char* apn, const char* username, const char* password, RadioAccessTechnologyType rat = CATNB, uint32_t band = BAND_20, bool restart = false);

  /*
   * Disconnect from the network
   *
   * return: one value of wl_status_t enum
   */
  int disconnect(void);

  /*
   * Reset internal state machine in order to be ready to reconnect again.
   */
  void end(void);

  /*
   * Send AT+CFUN=1,1 command to trigger a software reset. To be called only after end();
   */
  void reset();

  /*
   * Send AT^SMSO="fast command to power off the modem. To be called only after end();
   */
  void off();

  /*
   * Change cellular state timeouts. Needs to be called before GSM.begin()
   */
  void setTimeout(unsigned long timeout);

  unsigned long getTime();
  unsigned long getLocalTime();

  bool setTime(unsigned long const epoch, int const timezone = 0);
  void enableCmux();
  bool isCmuxEnable();
#if MBED_CONF_MBED_TRACE_ENABLE
  void trace(Stream& stream);
  void setTraceLevel(int trace_level, bool timestamp = false, bool at_trace = false);
#endif
  int ping(const char* hostname, int ttl = 5000);
  int ping(const String& hostname, int ttl = 5000);
  int ping(IPAddress host, int ttl = 5000);
  bool isConnected();

  friend class GSMClient;
  friend class GSMUDP;

  NetworkInterface* getNetwork();

private:
  const char* _pin = nullptr;
  const char* _apn = nullptr;
  const char* _username = nullptr;
  const char* _password = nullptr;
  bool _cmuxGSMenable = _CMUX_ENABLE;
  RadioAccessTechnologyType _rat;
  FrequencyBand _band;
  NetworkInterface* gsm_if = nullptr;
  mbed::CellularContext* _context = nullptr;
  mbed::CellularDevice* _device = nullptr;
  bool _at_debug = false;
  unsigned long _timeout = 1000;

  /* Internal cellular state machine retries. Values are in seconds.
   * This array also defines the maximum number of retries to CELLULAR_RETRY_ARRAY_SIZE
   */
  const uint16_t _retry_timeout[CELLULAR_RETRY_ARRAY_SIZE] = {1, 2, 4, 8, 8, 8, 8, 8, 8, 8};

  static constexpr int RSSI_UNKNOWN = 99;
  static const char * const sim_state_str[];
  static const char * const reg_type_str[];
  static const char * const rat_str[];
  static const char * const state_str[];
  static const char * const event_str[];
  static const char * getRATString(const mbed::CellularNetwork::RadioAccessTechnology rat);
  static const char * getStateString(const mbed::CellularStateMachine::CellularState state);
  static const char * getEventString(const cellular_event_status event);
  static const char * getSIMStateString(const mbed::CellularDevice::SimState state);
  static const char * getRegistrationStateString(const mbed::CellularNetwork::RegistrationStatus state);
  void onStatusChange(nsapi_event_t ev, intptr_t in);

  void hardwareReset();
  void on();
};

}

extern GSMClass GSM;

#include "GSMClient.h"
#include "GSMSSLClient.h"
#include "GSMUdp.h"

#endif
