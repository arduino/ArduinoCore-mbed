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

#include "SocketHelpers.h"
#include "CellularContext.h"

#include "GEMALTO_CINTERION.h"
#include "GENERIC_AT3GPP.h"

#include "drivers/BufferedSerial.h"

#define MBED_CONF_GEMALTO_CINTERION_TX    PA_0
#define MBED_CONF_GEMALTO_CINTERION_RX    PI_9
#define MBED_CONF_GEMALTO_CINTERION_RTS   PI_10
#define MBED_CONF_GEMALTO_CINTERION_CTS   PI_13
#define MBED_CONF_APP_SOCK_TYPE           1

namespace arduino {

typedef void* (*voidPrtFuncPtr)(void);

class GSMClass : public MbedSocketClass {
public:

  GSMClass()
    : _rat(CATNB) {}

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

  void debug(Stream& stream);
  void beginGNSS(mbed::Callback<void(char*)> gnss_cb);
  void endGNSS();
  void startGNSS();
  void stopGNSS();
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


class PTYSerial: public FileHandle {
public:
  PTYSerial(CMUXClass* parent) : _parent(parent) {};
  int populate_rx_buffer(char* buf, size_t sz) {
    // TO BE IMPLEMENTED
  }
  int write(char* buf, size_t sz) {
    _parent->populate_tx_buffer(buf, sz, this != _parent->get_serial(0));
  }
}

class CMUXClass : {
public:
  //CMUXClass(BufferedSerial* hw_serial)
  CMUXClass(FileHandle* hw_serial) : _hw_serial(hw_serial) {

    _gsm_serial = new PTYSerial(this);
    _gps_serial = new PTYSerial(this);

    reader_thd.start(mbed::callback(this, &CMUXClass::read));
    writer_thd.start(mbed::callback(this, &CMUXClass::write));
    _hw_serial.attach(mbed::callback(this, &CMUXClass::on_rx), mbed::SerialBase::RxIrq);
  }

  void on_rx() {
    while(_hw_serial->readable()) {
      char c;
      core_util_critical_section_enter();
      _hw_serial->read(&c, 1);
      rx_buffer.push(c);
    }
    osSignalSet(reader_thd.get_id(), 0xA);
  }

  void read() {
    while (1) {
      osSignalWait(0, osWaitForever);
      char temp_buf[256];
      size_t howMany = rx_buffer.size();
      rx_buffer.pop(temp_buf, howMany);
      size_t i = 0;
      while (i < howMany) {
        char payload[256];
        int frame_id;
        // cmux_handle_frame increments temp_buf
        auto ret = cmux_handle_frame(temp_buf, howMany, final_buf, &frame_id);
        if (ret <= 0) {
          // push again pop-ped data in rx_buffer and break
          rx_buffer.push(temp_buf, howMany - actualPosition);
          break;
        }
        if (frame_id == 0) {
          _gsm_serial->populate_rx_buffer(final_buf, ret);
        }
        if (frame_id == 1) {
          _gps_serial->populate_rx_buffer(final_buf, ret);
        }
      }
    }
  }

  void write() {
    while (1) {
      auto ev = osSignalWait(0, osWaitForever);
      char payload[256];
      char frame[256];
      uint8_t frame_id = ev.value.v;
      size_t howMany = tx_buffer.size();
      tx_buffer.pop(temp_buf, howMany);
      int ret = cmux_write_buffer(payload, howMany, frame_id, frame);
      if (ret > 0) {
        _hw_serial->write(frame, ret);
      }
    }
  }

  int populate_tx_buffer(char* buf, size_t sz, uint8_t id) {
    tx_buffer.push(buf, sz);
    osSignalSet(writer_thd.get_id(), id);
  }

  static CMUXClass *get_default_instance();

  FileHandle* get_serial(int index) {
    if (index == 0) {
      return _gsm_serial;
    }
    if (index == 1) {
      return _gps_serial;
    }
    return nullptr;
  }

private:
  FileHandle* _hw_serial = nullptr;
  FileHandle* _gsm_serial = nullptr;
  FileHandle* _gps_serial = nullptr;
  CircularBuffer<char, 1024> rx_buffer;
  CircularBuffer<char, 1024> tx_buffer;
  rtos::Thread reader_thd = rtos::Thread{osPriorityNormal, 4096, nullptr, "CMUXt1"};
  rtos::Thread writer_thd = rtos::Thread{osPriorityNormal, 4096, nullptr, "CMUXt2"};;
};

}

extern GSMClass GSM;

#include "GSMClient.h"
#include "GSMUdp.h"

#endif
