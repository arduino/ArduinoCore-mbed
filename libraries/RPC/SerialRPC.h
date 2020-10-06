#include "RPC_internal.h"
#include "Arduino.h"

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

	using Print::write; // pull in write(str) and write(buf, size) from Print

	void onWrite(std::vector<uint8_t> vec) {
	  for (int i = 0; i < vec.size(); i++) {
	  	rx_buffer.store_char(vec[i]);
	  }
	  // call attached function
	  if (_rx) {
	    _rx.call();
	  }
	}

	size_t write(uint8_t c) {
		write(&c, 1);
	}

	size_t write(uint8_t* buf, size_t len) {
		tx_buffer.clear();
		for (int i=0; i<len; i++) {
			tx_buffer.push_back(buf[i]);
		}
		RPC1.call("on_write", tx_buffer);
	}

	int begin() {
		RPC1.begin();
		RPC1.bind("on_write", mbed::callback(this, &SerialRPCClass::onWrite));
	}

	operator bool() {
		return RPC1;
	}

    void attach(void (*fptr)(void))
    {
        if (fptr != NULL) {
            _rx = mbed::Callback<void()>(fptr);
        }
    }

private:
   	mbed::Callback<void()> _rx;
	RingBufferN<1024> rx_buffer;
	std::vector<uint8_t> tx_buffer;
};
}

extern arduino::SerialRPCClass SerialRPC;