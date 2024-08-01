// Copyright (c) 2024 Arduino SA
// SPDX-License-Identifier: MPL-2.0
#include "Arduino.h"
#include "mbed.h"
#include "rpc/dispatcher.h"

//forward declaration
namespace arduino {
class RPCClass;
}

namespace rpc {
class client {

  public:
    template <typename... Args>
    RPCLIB_MSGPACK::object_handle call(std::string const &func_name,
                                       Args... args) {
      LOG_DEBUG("Call function {} and wait for result", func_name);

      callThreadId = osThreadGetId();

      auto args_obj = std::make_tuple(args...);
      auto call_obj = std::make_tuple(
                        static_cast<uint8_t>(client::request_type::call), (int32_t)callThreadId, func_name,
                        args_obj);

      auto buffer = new RPCLIB_MSGPACK::sbuffer;
      RPCLIB_MSGPACK::pack(*buffer, call_obj);

      write(buffer);

      auto e = osSignalWait(0, timeout);
      delete buffer;

      if (e.status != osEventSignal) {
        has_timed_out = true;
      }

      RPCLIB_MSGPACK::object_handle q = std::move(result);
      return q;
    }

    //! \brief Sends a notification with the given name and arguments (if any).
    //! \param func_name The name of the notification to call.
    //! \param args The arguments to pass to the function.
    //! \note This function returns when the notification is written to the
    //! socket.
    //! \tparam Args THe types of the arguments.
    template <typename... Args>
    void send(std::string const &func_name, Args... args) {
      LOG_DEBUG("Call function {} and forget", func_name);

      callThreadId = osThreadGetId();

      auto args_obj = std::make_tuple(args...);
      auto call_obj = std::make_tuple(
                        static_cast<uint8_t>(client::request_type::notification), func_name, args_obj);

      auto buffer = new RPCLIB_MSGPACK::sbuffer;
      RPCLIB_MSGPACK::pack(*buffer, call_obj);

      write(buffer);
      delete buffer;
    }

    void setTimeout(uint32_t milliseconds) {
      timeout = milliseconds;
    }

    bool timedOut() {
      return has_timed_out;
    }

  protected:
    osThreadId callThreadId;
    friend class arduino::RPCClass;
    RPCLIB_MSGPACK::object_handle result;

    uint32_t timeout = osWaitForever;
    bool has_timed_out = false;

  private:
    enum class request_type { call = 0, notification = 2 };;

    void write(RPCLIB_MSGPACK::sbuffer *buffer);
    void getResult(RPCLIB_MSGPACK::object_handle& res);
};
}
