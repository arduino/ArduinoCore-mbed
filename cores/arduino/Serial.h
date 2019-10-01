/*
  Serial.h - wrapper over mbed RawSerial
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2018-2019 Arduino SA

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/

#include "api/RingBuffer.h"
#include "Arduino.h"

#ifdef __cplusplus

#ifndef __ARDUINO_UART_IMPLEMENTATION__
#define __ARDUINO_UART_IMPLEMENTATION__

namespace arduino {

class UART : public HardwareSerial {
	public:
		UART(int _tx, int _rx, int _rts, int _cts) : tx((PinName)_tx), rx((PinName)_rx), rts((PinName)_rts), cts((PinName)_cts) {};
		void begin(unsigned long);
		void begin(unsigned long baudrate, uint16_t config);
		void end();
		int available(void);
		int peek(void);
		int read(void);
		void flush(void);
		size_t write(uint8_t c);
		#ifdef DEVICE_SERIAL_ASYNCH
		size_t write(const uint8_t*, size_t);
		#endif
		using Print::write; // pull in write(str) and write(buf, size) from Print
		operator bool();

	private:
		void on_rx();
		void block_tx(int);
		bool _block;
		// See https://github.com/ARMmbed/mbed-os/blob/f5b5989fc81c36233dbefffa1d023d1942468d42/targets/TARGET_NORDIC/TARGET_NRF5x/TARGET_NRF52/serial_api.c#L76
		const size_t WRITE_BUFF_SZ = 32;
		mbed::RawSerial* _serial = NULL;
		PinName tx, rx, rts, cts;
		RingBufferN<256> rx_buffer;
		uint8_t intermediate_buf[4];
};
}

extern arduino::UART UART1;
extern arduino::UART UART2;
extern arduino::UART UART3;

#endif
#endif
