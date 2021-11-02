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
                        static_cast<uint8_t>(client::request_type::request), (const int)callThreadId, func_name,
                        args_obj);

      auto buffer = new RPCLIB_MSGPACK::sbuffer;
      RPCLIB_MSGPACK::pack(*buffer, call_obj);

      send_msgpack(buffer);

      osSignalWait(0, osWaitForever);

      //getResult(result);

      delete buffer;

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

      auto args_obj = std::make_tuple(args...);
      auto call_obj = std::make_tuple(
                        static_cast<uint8_t>(client::request_type::request_no_answer), func_name,
                        args_obj);

      auto buffer = new RPCLIB_MSGPACK::sbuffer;
      RPCLIB_MSGPACK::pack(*buffer, call_obj);

      send_msgpack(buffer);
      delete buffer;
    }

  protected:
    osThreadId callThreadId;
    friend class arduino::RPCClass;
    RPCLIB_MSGPACK::object_handle result;

  private:
    enum class request_type { raw = 1, request = 2, request_no_answer = 3,  response = 4 };

    void send_msgpack(RPCLIB_MSGPACK::sbuffer *buffer);
    void getResult(RPCLIB_MSGPACK::object_handle& res);
};
}
