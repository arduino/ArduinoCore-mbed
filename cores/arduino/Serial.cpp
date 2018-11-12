#include "Arduino.h"

#ifdef Serial
#undef Serial
#endif

using namespace arduino;

static EventQueue queue(4 * EVENTS_EVENT_SIZE);
static rtos::Thread t;

void UART::begin(unsigned long baudrate, uint16_t config) {
	begin(baudrate);
	_serial->format();
}

void UART::begin(unsigned long baudrate) {
	if (_serial == NULL) {
		_serial = new mbed::Serial(tx, rx, baudrate);
		t.start(mbed::callback(&queue, &EventQueue::dispatch_forever));
	}
	//_serial->attach(queue.event(mbed::callback(this, &UART::on_rx)));
}

void UART::on_rx() {
	while (_serial->readable() > 0) {
		int ret = _serial->getc();
		rx_buffer.store_char(ret);
	}
}

void UART::end() {
	if (_serial != NULL) {
		delete _serial;
	}
}

int UART::available() {
	on_rx();
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