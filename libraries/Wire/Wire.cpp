/*
  Wire.cpp - wrapper over mbed I2C / I2CSlave
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

#include "Wire.h"

arduino::MbedI2C::MbedI2C(int sda, int scl) : _sda(sda), _scl(scl), usedTxBuffer(0) {}

void arduino::MbedI2C::begin() {
	master = new mbed::I2C((PinName)_sda, (PinName)_scl);
}

void arduino::MbedI2C::begin(uint8_t slaveAddr) {
#ifdef DEVICE_I2CSLAVE
	slave = new mbed::I2CSlave((PinName)_sda, (PinName)_scl);
	slave->address(slaveAddr);
#endif
}

void arduino::MbedI2C::end() {
	if (master != NULL) {
		delete master;
	}
#ifdef DEVICE_I2CSLAVE
	if (slave != NULL) {
		delete slave;
	}
#endif
}

void arduino::MbedI2C::setClock(uint32_t freq) {
	if (master != NULL) {
		master->frequency(freq);
	}
#ifdef DEVICE_I2CSLAVE
	if (slave != NULL) {
		slave->frequency(freq);
	}
#endif
}

void arduino::MbedI2C::beginTransmission(uint8_t address) {
	_address = address << 1;
	usedTxBuffer = 0;
}

uint8_t arduino::MbedI2C::endTransmission(bool stopBit) {
	if (master->write(_address, (const char *) txBuffer, usedTxBuffer, !stopBit) == 0) return 0;
	return 2;
}

uint8_t arduino::MbedI2C::endTransmission(void) {
	return endTransmission(true);
}

uint8_t arduino::MbedI2C::requestFrom(uint8_t address, size_t len, bool stopBit) {
	char buf[256];
	int ret = master->read(address << 1, buf, len, !stopBit);
	if (ret != 0) {
		return 0;
	}
	for (size_t i=0; i<len; i++) {
		rxBuffer.store_char(buf[i]);
	}
	return len;
}

uint8_t arduino::MbedI2C::requestFrom(uint8_t address, size_t len) {
	return requestFrom(address, len, true);
}

size_t arduino::MbedI2C::write(uint8_t data) {
	if (usedTxBuffer == 256) return 0;
	txBuffer[usedTxBuffer++] = data;
	return 1;
}

size_t arduino::MbedI2C::write(uint8_t* data, int len) {
	if (usedTxBuffer + len > 256) len = 256 - usedTxBuffer;
	memcpy(txBuffer + usedTxBuffer, data, len);
	usedTxBuffer += len;
	return len;
}

int arduino::MbedI2C::read() {
	if (rxBuffer.available()) {
		return rxBuffer.read_char();
	}
	return 0;
}

int arduino::MbedI2C::available() {
	return rxBuffer.available();
}

int arduino::MbedI2C::peek() {
	return rxBuffer.peek();
}

void arduino::MbedI2C::flush() {
}


void arduino::MbedI2C::onReceive(void(*)(int)) {}
void arduino::MbedI2C::onRequest(void(*)(void)) {}


#if DEVICE_I2C > 0
arduino::MbedI2C Wire(I2C_SDA, I2C_SCL);
#endif
#if DEVICE_I2C > 1
arduino::MbedI2C Wire1(I2C_SDA1, I2C_SCL1);;
#endif