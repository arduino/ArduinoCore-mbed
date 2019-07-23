#include "Arduino.h"

#ifdef __cplusplus

#ifndef __ARDUINO_CDC_IMPLEMENTATION__
#define __ARDUINO_CDC_IMPLEMENTATION__

#ifdef SERIAL_CDC
#include "USBSerial.h"

namespace arduino {

namespace internal {
extern USBSerial _serial;
}

static void usbPortChanged(int baud, int bits, int parity, int stop) {
  if (baud == 1200 && internal::_serial.connected()) {
    _ontouch1200bps_();
  }
}

class CDC : public HardwareSerial {
	public:
		CDC() {}
		void begin(unsigned long) {
			internal::_serial.connect();
			internal::_serial.attach(usbPortChanged);
			internal::_serial.attach(mbed::callback(this, &CDC::onInterrupt));
		}
		void begin(unsigned long baudrate, uint16_t config) {
			begin(baudrate);
		}
		void end() {
			internal::_serial.deinit();
		}
		int available(void) {
			return rx_buffer.available();
		}
		int peek(void) {
			return rx_buffer.peek();
		}
		int read(void) {
			return rx_buffer.read_char();
		}
		void flush(void) {}
		size_t write(uint8_t c) {
			if (!(internal::_serial.connected())) {
				return 0;
			}
			return internal::_serial._putc(c);
		}
		size_t write(const uint8_t* buf, size_t size) {
			if (!(internal::_serial.connected())) {
				return 0;
			}
			return internal::_serial.send((uint8_t*)buf, size);
		}
		using Print::write; // pull in write(str) and write(buf, size) from Print
		operator bool() {
			return internal::_serial.connected();
		}
		USBSerial& mbed() {
			return internal::_serial;
		}
	private:
		RingBufferN<256> rx_buffer;
		void onInterrupt() {
			while (rx_buffer.availableForStore() && internal::_serial.available()) {
				rx_buffer.store_char(internal::_serial._getc());
			}
		}
};
}

extern arduino::CDC SerialUSB;

#endif
#endif
#endif
