#ifdef __cplusplus

#ifndef __ARDUINO_RPC_IMPLEMENTATION__
#define __ARDUINO_RPC_IMPLEMENTATION__

#include "Arduino.h"

extern "C" {
#define boolean   boolean_t
#include "openamp.h"
#undef boolean
#define boolean   bool
}

enum service_request_code_t {
  REQUEST_REBOOT = 0x7F7F7F7F,
  CALL_FUNCTION  = 0x12345678,
};

typedef struct _service_request {
  enum service_request_code_t code;
  size_t length;
  size_t parameters;
  uint8_t* data;
} service_request;

namespace arduino {

class RPC : public Stream {
	public:
		RPC() {};
		int begin();
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
		size_t write(uint8_t c);
		size_t write(const uint8_t*, size_t);
		int request(service_request* s);
		using Print::write; // pull in write(str) and write(buf, size) from Print
		operator bool() {
			return initialized;
		}

	private:
		RingBufferN<256> rx_buffer;
		bool initialized = false;
		struct rpmsg_endpoint rp_endpoints[4];
		static int rpmsg_recv_service_callback(struct rpmsg_endpoint *ept, void *data,
                                       size_t len, uint32_t src, void *priv);
		static int rpmsg_recv_raw_callback(struct rpmsg_endpoint *ept, void *data,
                                       size_t len, uint32_t src, void *priv);
		events::EventQueue eventQueue;
		mbed::Ticker ticker;
		rtos::Thread* eventThread;
};
}

extern arduino::RPC RPC1;

#endif
#endif