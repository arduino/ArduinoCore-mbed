/*
  Serial.cpp - wrapper over mbed RawSerial
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

#include "Arduino.h"

#ifdef Serial
#undef Serial
#endif

using namespace arduino;

void UART::begin(unsigned long baudrate, uint16_t config) {
	begin(baudrate);
	_serial->format();
}

void UART::begin(unsigned long baudrate) {
	if (_serial == NULL) {
		_serial = new mbed::RawSerial(tx, rx, baudrate);
	}
	if (rts != NC) {
		_serial->set_flow_control(mbed::SerialBase::Flow::RTSCTS, rts, cts);
	}
	_serial->attach(mbed::callback(this, &UART::on_rx), mbed::SerialBase::RxIrq);
}

void UART::on_rx() {
	while(_serial->readable()) {
		rx_buffer.store_char(_serial->getc());
	}
}

void UART::end() {
	if (_serial != NULL) {
		delete _serial;
	}
}

int UART::available() {
	return rx_buffer.available();
}

int UART::peek() {
	return rx_buffer.peek();
}

int UART::read() {
	return rx_buffer.read_char();
}

void UART::flush() {

}

size_t UART::write(uint8_t c) {
	while (!_serial->writeable()) {}
	return _serial->putc(c);
}

/*
size_t UART::write(const uint8_t* c, size_t len) {
	//while (!_serial->writeable()) {}
	return _serial->puts(c, len);
}
*/
UART::operator bool() {
	return 1;
}

#if SERIAL_HOWMANY > 0

#ifdef SERIAL1_RTS
UART UART1(SERIAL1_TX, SERIAL1_RX, SERIAL1_RTS, SERIAL1_CTS);
#else
UART UART1(SERIAL1_TX, SERIAL1_RX, NC, NC);
#endif

#if SERIAL_HOWMANY > 1

#ifdef SERIAL2_RTS
UART UART2(SERIAL2_TX, SERIAL2_RX, SERIAL2_RTS, SERIAL2_CTS);
#else
UART UART2(SERIAL2_TX, SERIAL2_RX, NC, NC);
#endif

#if SERIAL_HOWMANY > 2

#ifdef SERIAL3_RTS
UART UART1(SERIAL3_TX, SERIAL3_RX, SERIAL3_RTS, SERIAL3_CTS);
#else
UART UART1(SERIAL3_TX, SERIAL3_RX, NC, NC);
#endif

#endif
#endif
#endif