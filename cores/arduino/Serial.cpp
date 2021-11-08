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
#include "pinDefinitions.h"
#include "Serial.h"
#include "drivers/UnbufferedSerial.h"
#if defined(SERIAL_CDC)
#include "USB/PluggableUSBSerial.h"
#endif

#ifdef Serial
#undef Serial
#endif

using namespace arduino;

struct _mbed_serial {
	mbed::UnbufferedSerial* obj;
};

UART::UART(int tx, int rx, int rts, int cts) {
	_tx = digitalPinToPinName(tx);
	_rx = digitalPinToPinName(rx);
	if (rts >= 0) {
		_rts = digitalPinToPinName(rts);
	} else {
		_rts = NC;
	}
	if (cts >= 0) {
		_cts = digitalPinToPinName(cts);
	} else {
		_cts = NC;
	}
}

void UART::begin(unsigned long baudrate, uint16_t config) {

#if defined(SERIAL_CDC)
	if (is_usb) {
		return;
	}
#endif
	begin(baudrate);
	int bits = 8;
	mbed::SerialBase::Parity parity = mbed::SerialBase::None;
	int stop_bits = 1;

	switch (config & SERIAL_DATA_MASK) {
		case SERIAL_DATA_7:
			bits = 7;
			break;
		case SERIAL_DATA_8:
			bits = 8;
			break;
/*
		case SERIAL_DATA_9:
			bits = 9;
			break;
*/
	}

	switch (config & SERIAL_STOP_BIT_MASK) {
		case SERIAL_STOP_BIT_1:
			stop_bits = 1;
			break;
		case SERIAL_STOP_BIT_2:
			stop_bits = 2;
			break;
	}

	switch (config & SERIAL_PARITY_MASK) {
		case SERIAL_PARITY_EVEN:
			parity = mbed::SerialBase::Even;
			break;
		case SERIAL_PARITY_ODD:
			parity = mbed::SerialBase::Odd;
			break;
		case SERIAL_PARITY_NONE:
			parity = mbed::SerialBase::None;
			break;
	}

	_serial->obj->format(bits, parity, stop_bits);
}

void UART::begin(unsigned long baudrate) {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return;
	}
#endif
	if (_serial == NULL) {
		_serial = new mbed_serial;
		_serial->obj = NULL;
	}
	if (_serial->obj == NULL) {
		_serial->obj = new mbed::UnbufferedSerial(_tx, _rx, baudrate);
	} else {
		_serial->obj->baud(baudrate);
	}
	if (_rts != NC) {
		_serial->obj->set_flow_control(mbed::SerialBase::Flow::RTSCTS, _rts, _cts);
	}
	if (_serial->obj != NULL) {
		_serial->obj->attach(mbed::callback(this, &UART::on_rx), mbed::SerialBase::RxIrq);
	}
}

void UART::on_rx() {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return;
	}
#endif
	while(_serial->obj->readable()) {
		char c;
		core_util_critical_section_enter();
		_serial->obj->read(&c, 1);
		if (rx_buffer.availableForStore()) {
			rx_buffer.store_char(c);
		}
		core_util_critical_section_exit();
	}
}

void UART::end() {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return _SerialUSB.end();
	}
#endif
	if (_serial != NULL && _serial->obj != NULL) {
		delete _serial->obj;
		_serial->obj = NULL;
		delete _serial;
		_serial = NULL;
	}
	rx_buffer.clear();
}

int UART::available() {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return _SerialUSB.available();
	}
#endif
	core_util_critical_section_enter();
	int c = rx_buffer.available();
	core_util_critical_section_exit();
	return c;
}

int UART::peek() {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return _SerialUSB.peek();
	}
#endif
	core_util_critical_section_enter();
	int c = rx_buffer.peek();
	core_util_critical_section_exit();
	return c;
}

int UART::read() {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return _SerialUSB.read();
	}
#endif
	core_util_critical_section_enter();
	int c = rx_buffer.read_char();
	core_util_critical_section_exit();
	return c;
}

void UART::flush() {
#if defined(SERIAL_CDC)
	if (is_usb) {
		while(!_SerialUSB.writeable());
	} else {
		while(!_serial->obj->writeable());
	}
#else
	while(!_serial->obj->writeable());
#endif
}

size_t UART::write(uint8_t c) {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return _SerialUSB.write(c);
	}
#endif
	while (!_serial->obj->writeable()) {}
	int ret = _serial->obj->write(&c, 1);
	return ret == -1 ? 0 : 1;
}

size_t UART::write(const uint8_t* c, size_t len) {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return _SerialUSB.write(c, len);
	}
#endif
	while (!_serial->obj->writeable()) {}
	_serial->obj->set_blocking(true);
	int ret = _serial->obj->write(c, len);
	return ret == -1 ? 0 : len;
}

void UART::block_tx(int _a) {
	_block = false;
}

UART::operator bool() {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return _SerialUSB;
	}
#endif
	return _serial != NULL && _serial->obj != NULL;
}

UART::operator mbed::FileHandle*() {
#if defined(SERIAL_CDC)
	if (is_usb) {
		return &_SerialUSB;
	}
#endif
	return _serial->obj;
}


#if defined(SERIAL_CDC)
	uint32_t UART::baud() {
		if (is_usb) {
			return _SerialUSB.baud();
		}
		return 0;
	}
	uint8_t UART::stopbits() {
		if (is_usb) {
			return _SerialUSB.stopbits();
		}
		return 0;
	}
	uint8_t UART::paritytype() {
		if (is_usb) {
			return _SerialUSB.paritytype();
		}
		return 0;
	}
	uint8_t UART::numbits() {
		if (is_usb) {
			return _SerialUSB.numbits();
		}
		return 0;
	}
	bool UART::dtr() {
		if (is_usb) {
			return _SerialUSB.dtr();
		}
		return false;
	}
	bool UART::rts() {
		if (is_usb) {
			return _SerialUSB.rts();
		}
		return false;
	}
#endif

#if defined(SERIAL_CDC)
UART _UART_USB_;
#endif

#if SERIAL_HOWMANY > 0

#ifdef SERIAL1_RTS
UART _UART1_(SERIAL1_TX, SERIAL1_RX, SERIAL1_RTS, SERIAL1_CTS);
#else
UART _UART1_(SERIAL1_TX, SERIAL1_RX, NC, NC);
#endif

#if SERIAL_HOWMANY > 1

#ifdef SERIAL2_RTS
UART _UART2_(SERIAL2_TX, SERIAL2_RX, SERIAL2_RTS, SERIAL2_CTS);
#else
UART _UART2_(SERIAL2_TX, SERIAL2_RX, NC, NC);
#endif

#if SERIAL_HOWMANY > 2

#ifdef SERIAL3_RTS
UART _UART3_(SERIAL3_TX, SERIAL3_RX, SERIAL3_RTS, SERIAL3_CTS);
#else
UART _UART3_(SERIAL3_TX, SERIAL3_RX, NC, NC);
#endif

#if SERIAL_HOWMANY > 3

#ifdef SERIAL4_RTS
UART _UART4_(SERIAL4_TX, SERIAL4_RX, SERIAL4_RTS, SERIAL4_CTS);
#else
UART _UART4_(SERIAL4_TX, SERIAL4_RX, NC, NC);
#endif

#endif
#endif
#endif
#endif
