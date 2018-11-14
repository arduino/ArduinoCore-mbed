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
		_serial = new mbed::Serial(tx, rx, baudrate);
	}
	_serial->read(intermediate_buf, 1, mbed::callback(this, &UART::on_rx));
}

void UART::on_rx(int howmany) {
	rx_buffer.store_char(intermediate_buf[0]);
	_serial->read(intermediate_buf, 1, mbed::callback(this, &UART::on_rx));
}

void UART::end() {
	if (_serial != NULL) {
		delete _serial;
	}
}

int UART::available() {
	//on_rx();
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

UART UART1(USBTX, USBRX);