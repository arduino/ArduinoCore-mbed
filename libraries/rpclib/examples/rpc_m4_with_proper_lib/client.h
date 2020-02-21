#include "rpc/dispatcher.h"
#include "Arduino.h"

extern volatile uint8_t* rpc_data;
extern volatile size_t rpc_len;
int signal_rpc_available(void *data, size_t len);

namespace rpc {
class myclient {

  public:
    template <typename... Args>
    RPCLIB_MSGPACK::object_handle call(std::string const &func_name,
                                       Args... args) {
      LOG_DEBUG("Sending notification {}", func_name);

      auto args_obj = std::make_tuple(args...);
      auto call_obj = std::make_tuple(
                        static_cast<uint8_t>(myclient::request_type::call), 0, func_name,
                        args_obj);

      auto buffer = new RPCLIB_MSGPACK::sbuffer;
      RPCLIB_MSGPACK::pack(*buffer, call_obj);

      post(buffer);

      // wait answer and return it after unpacking
      // return ????
    }

    template <typename... Args>
    RPCLIB_MSGPACK::object_handle fake_call(std::string const &func_name,
                                       Args... args) {
      LOG_DEBUG("Sending notification {}", func_name);

      auto args_obj = std::make_tuple(args...);
      auto call_obj = std::make_tuple(
                        static_cast<uint8_t>(myclient::request_type::call), 0, func_name,
                        args_obj);

      auto buffer = new RPCLIB_MSGPACK::sbuffer;
      RPCLIB_MSGPACK::pack(*buffer, call_obj);

      signal_rpc_available(buffer->data(), buffer->size());

      delete buffer;

      RPCLIB_MSGPACK::object_handle b;
      return b;
      // wait answer and return it after unpacking
      // return ????
    }

    //! \brief Sends a notification with the given name and arguments (if any).
    //! \param func_name The name of the notification to call.
    //! \param args The arguments to pass to the function.
    //! \note This function returns when the notification is written to the
    //! socket.
    //! \tparam Args THe types of the arguments.
    template <typename... Args>
    void send(std::string const &func_name, Args... args) {
      LOG_DEBUG("Sending notification {}", func_name);

      auto args_obj = std::make_tuple(args...);
      auto call_obj = std::make_tuple(
                        static_cast<uint8_t>(myclient::request_type::notification), func_name,
                        args_obj);

      auto buffer = new RPCLIB_MSGPACK::sbuffer;
      RPCLIB_MSGPACK::pack(*buffer, call_obj);

      post(buffer);
    }

  private:
    enum class request_type { call = 0, notification = 2 };

    void post(RPCLIB_MSGPACK::sbuffer *buffer) {
      RPC1.write(ENDPOINT_SERVICE, (const uint8_t*)buffer->data(), buffer->size());
    }
};
}
