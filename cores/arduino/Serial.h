#include "api/RingBuffer.h"
#include "Arduino.h"

#ifdef __cplusplus

#ifndef __ARDUINO_UART_IMPLEMENTATION__
#define __ARDUINO_UART_IMPLEMENTATION__

namespace arduino {

class UART : public HardwareSerial {
	public:
		UART(int _tx, int _rx) : tx((PinName)_tx), rx((PinName)_rx) {};
		void begin(unsigned long);
		void begin(unsigned long baudrate, uint16_t config);
		void end();
		int available(void);
		int peek(void);
		int read(void);
		void flush(void);
		size_t write(uint8_t c);
		//size_t write(const uint8_t*, size_t);
		using Print::write; // pull in write(str) and write(buf, size) from Print
		operator bool();

	private:
		void on_rx();
		mbed::RawSerial* _serial = NULL;
		PinName tx, rx;
		RingBufferN<256> rx_buffer;
		uint8_t intermediate_buf[4];
};
}

extern arduino::UART UART1;
extern arduino::UART UART2;
extern arduino::UART UART3;
#ifdef SERIAL_CDC
#include "USBSerial.h"
extern USBSerial SerialUSB;
#endif

#endif
#endif