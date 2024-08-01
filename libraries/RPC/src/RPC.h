// Copyright (c) 2024 Arduino SA
// SPDX-License-Identifier: MPL-2.0
#ifdef __cplusplus

#ifndef __ARDUINO_RPC_IMPLEMENTATION__
#define __ARDUINO_RPC_IMPLEMENTATION__

// msgpack overrides BIN symbol, undefine it 
#ifdef BIN
#define _BIN BIN
#undef BIN
#define BIN BIN_MSGPACK
#endif
#include "rpclib.h"
#include "rpc/dispatcher.h"
#include "RPC_client.h"
#ifdef _BIN
#undef BIN
#define BIN _BIN
#endif

extern "C" {
#ifdef ATOMIC_FLAG_INIT
#undef ATOMIC_FLAG_INIT
#endif
#ifdef ATOMIC_VAR_INIT
#undef ATOMIC_VAR_INIT
#endif
#define boolean   boolean_t
#include "openamp.h"
#include "arduino_openamp.h"
#undef boolean
#ifdef bool
#define boolean   bool
#endif
}

#include "mbed.h"

namespace arduino {
class RPCClass : public Stream, public rpc::detail::dispatcher {
    public:
        RPCClass() {
            for (int i = 0; i< 10; i++) {
                clients[i] = NULL;
            }
        }
        int begin();
        void end() {

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
        void flush(void) {

        }
        uint32_t cpu_id() {
            return HAL_GetCurrentCPUID();
        }
        size_t write(uint8_t c);
        size_t write(const uint8_t *buf, size_t len, bool raw = true);

        using Print::write; // pull in write(str) and write(buf, size) from Print
        operator bool() {
            return initialized;
        }

        void attach(void (*fptr)(const uint8_t *buf, size_t len)) {
            if (fptr != NULL) {
                raw_callback = mbed::Callback<void(const uint8_t *buf, size_t len)>(fptr);
            }
        }

        template <typename... Args>
        void send(std::string const &func_name, Args... args) {
            auto client = new rpc::client();
            client->send(func_name, args...);
            delete client;
        }

        void setTimeout(uint32_t milliseconds) {
            _timeout = milliseconds;
        }

        template <typename... Args>
        RPCLIB_MSGPACK::object_handle call(std::string const &func_name, Args... args) {
                // find a free spot in clients[]
                // create new object
                // protect this with mutex

                int i = 0;
                for (i=0; i<10; i++) {
                    if (clients[i] == NULL) {
                        clients[i] = new rpc::client();
                        break;
                    }
                }

                clients[i]->setTimeout(_timeout);
                has_timed_out = false;

                // thread start and client .call
                clients[i]->call(func_name, args...);

                if (clients[i]->timedOut()) {
                    has_timed_out = true;
                }
                RPCLIB_MSGPACK::object_handle ret = std::move(clients[i]->result);

                delete clients[i];
                clients[i] = NULL;
                return ret;
            }

        bool timedOut() {
            return has_timed_out;
        }

        rpc::client* clients[10];
        RingBufferN<512> rx_buffer;
        mbed::Callback<void(const uint8_t *buf, size_t len)> raw_callback;

    private:
        bool initialized = false;
        uint32_t _timeout = osWaitForever;
        bool has_timed_out = false;
        rtos::Thread* eventThread;
        RPCLIB_MSGPACK::unpacker unpacker;

        static void new_service_cb(struct rpmsg_device *rdev, const char *name, uint32_t dest);
        static int rpmsg_recv_callback(struct rpmsg_endpoint *ept, void *data, size_t len, uint32_t src, void *priv);
        void request(uint8_t *buf, size_t len);
        void response(uint8_t *buf, size_t len);
};
}

extern arduino::RPCClass RPC;

#endif
#endif
