#ifdef __cplusplus
#ifdef TARGET_STM32H747_CM4

#ifndef __ARDUINO_RPC_IMPLEMENTATION__
#define __ARDUINO_RPC_IMPLEMENTATION__

#include "Arduino.h"

extern "C" {
#define boolean   boolean_t
#include "openamp.h"
#undef boolean
#ifdef bool
#define boolean   bool
#endif
}

enum endpoints_t {
	ENDPOINT_SERVICE = 0,
	ENDPOINT_RAW
};

enum service_request_code_t {
  REQUEST_REBOOT = 0x7F7F7F7F,
  CALL_FUNCTION  = 0x12345678,
};

typedef struct _service_request {
  enum service_request_code_t code;
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
		size_t write(enum endpoints_t ep, const uint8_t* buf, size_t len);

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
#endif
