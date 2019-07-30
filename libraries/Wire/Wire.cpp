#include "Wire.h"

arduino::MbedI2C::MbedI2C(int sda, int scl) : _sda(sda), _scl(scl) {}

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
	txBuffer.clear();
}

uint8_t arduino::MbedI2C::endTransmission(bool stopBit) {
	char buf[256];
	int len = txBuffer.available();
	for (int i=0; i<len; i++) {
		buf[i] = txBuffer.read_char();
	}
	master->write(_address, buf, len, !stopBit);
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
	txBuffer.store_char(data);
}

size_t arduino::MbedI2C::write(uint8_t* data, int len) {
	for (int i=0; i<len; i++) {
		write(data[i]);
	}
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