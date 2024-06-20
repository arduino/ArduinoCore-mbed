/*
  GPS.h - Library for GSM on mbed platforms.
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

#ifndef GPS_h
#define GPS_h

#include "api/HardwareSerial.h"
#include "Arduino.h"
#include "CMUXClass.h"
#include "PTYSerial.h"
#include "GSM.h"

namespace arduino {

class GPSClass : public HardwareSerial {
public:

  GPSClass();
  ~GPSClass();

  void begin(unsigned long baudrate = 115200);
  void begin(unsigned long baudrate, uint16_t config);
  void end(void);
  int available(void);
  int peek(void);
  int read(void);
  void flush(void);
  size_t write(uint8_t);
  size_t write(char * buffer);
  size_t write(char * buffer, size_t sz);
  operator bool();

protected:
  bool checkGNSSEngine(const char * prefix);
  void readAndPrint();
  void readAndDrop();

  mbed::CircularBuffer<char, 1500U> * _rxbuf;
  events::EventQueue _queue;
private:
  PTYSerial * _serial;
  bool _engine = false;
};

}

extern GPSClass GPS;

#endif
