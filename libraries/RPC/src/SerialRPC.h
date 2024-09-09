// Copyright (c) 2024 Arduino SA
// SPDX-License-Identifier: MPL-2.0
#ifndef __SERIAL_RPC__
#define __SERIAL_RPC__

#include "Arduino.h"
#include <vector>

namespace arduino {

class SerialRPCClass : public Stream {

public:
	SerialRPCClass() {};
	void end() {};
	int available(void) {
		return rx_buffer.available();
	};
	int peek(void) {
		return rx_buffer.peek();
	}
	int read(void) {
		return rx_buffer.read_char();
	}
	void flush(void) {};

	void onWrite(std::vector<uint8_t>& vec) {
	  for (size_t i = 0; i < vec.size(); i++) {
	  	rx_buffer.store_char(vec[i]);
	  }
	  // call attached function
	  if (_rx) {
	    _rx.call();
	  }
	}

	int begin(long unsigned int = 0, uint16_t = 0);
	size_t write(uint8_t* buf, size_t len);

	size_t write(uint8_t c) {
		return write(&c, 1);
	}

	size_t write(const uint8_t* buf, size_t len) override {
		return write((uint8_t*)buf, len);
	}
	size_t write(const char* buf, size_t len) {
		return write((uint8_t*)buf, len);
	}
	size_t write(char* buf, size_t len) {
		return write((uint8_t*)buf, len);
	}

	using Print::write;

	operator bool();

    void attach(void (*fptr)(void))
    {
        if (fptr != NULL) {
            _rx = mbed::Callback<void()>(fptr);
        }
    }

private:
   	mbed::Callback<void()> _rx;
	RingBufferN<1024> rx_buffer;
	std::vector<char> tx_buffer;
};
}

extern arduino::SerialRPCClass SerialRPC;

#endif
