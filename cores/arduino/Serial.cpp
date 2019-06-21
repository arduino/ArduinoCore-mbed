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
UART UART1(SERIAL1_TX, SERIAL1_RX);
#if SERIAL_HOWMANY > 1
UART UART2(SERIAL2_TX, SERIAL2_RX);
#if SERIAL_HOWMANY > 2
UART UART3(SERIAL3_TX, SERIAL3_RX);
#endif
#endif
#endif