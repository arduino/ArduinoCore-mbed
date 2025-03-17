#include "MbedClient.h"

#ifndef SOCKET_TIMEOUT
#define SOCKET_TIMEOUT 1500
#endif

arduino::MbedClient::MbedClient()
  : _status(false),
    _timeout(SOCKET_TIMEOUT) {
}

uint8_t arduino::MbedClient::status() {
  return (_status && (getNetwork()->get_connection_status() < NSAPI_STATUS_DISCONNECTED));
}


void arduino::MbedClient::readSocket() {
  uint8_t data[SOCKET_BUFFER_SIZE];

  while (sock != nullptr) {
    event->wait_any(0xFF, 100);
    int ret = NSAPI_ERROR_WOULD_BLOCK;
    do {
      mutex->lock();
      if (sock != nullptr && rxBuffer.availableForStore() == 0) {
        mutex->unlock();
        yield();
        continue;
      } else if (sock == nullptr) {
        goto cleanup;
      }
      ret = sock->recv(data, rxBuffer.availableForStore());
      if (ret < 0 && ret != NSAPI_ERROR_WOULD_BLOCK) {
        goto cleanup;
      }
      if (ret == NSAPI_ERROR_WOULD_BLOCK || ret == 0) {
        yield();
        mutex->unlock();
        continue;
      }
      for (int i = 0; i < ret; i++) {
        rxBuffer.store_char(data[i]);
      }
      mutex->unlock();
      _status = true;
    } while (ret == NSAPI_ERROR_WOULD_BLOCK || ret > 0);
  }
cleanup:
  _status = false;
  return;
}

void arduino::MbedClient::getStatus() {
  event->set(1);
}

void arduino::MbedClient::setSocket(Socket *_sock) {
  sock = _sock;
  configureSocket(sock);
}

void arduino::MbedClient::configureSocket(Socket *_s) {
  _s->set_blocking(false);
  _s->getpeername(&address);

  if (event == nullptr) {
    event = new rtos::EventFlags;
  }
  if (mutex == nullptr) {
    mutex = new rtos::Mutex;
  }
  mutex->lock();
  if (reader_th == nullptr) {
    reader_th = new rtos::Thread(osPriorityNormal, OS_STACK_SIZE, nullptr, "readSocket");
    reader_th->start(mbed::callback(this, &MbedClient::readSocket));
  }
  mutex->unlock();
  _s->sigio(mbed::callback(this, &MbedClient::getStatus));
  _status = true;
}

int arduino::MbedClient::connect(SocketAddress socketAddress) {
  // if a connection is aready ongoing, a disconnection must be enforced before starting another one
  stop();

  if (sock == nullptr) {
    sock = new TCPSocket();
    _own_socket = true;
  }
  if (sock == nullptr) {
    return 0;
  }

  if (static_cast<TCPSocket *>(sock)->open(getNetwork()) != NSAPI_ERROR_OK) {
    return 0;
  }

  nsapi_error_t returnCode = static_cast<TCPSocket *>(sock)->connect(socketAddress);
  int ret = 0;

  switch (returnCode) {
    case NSAPI_ERROR_IS_CONNECTED:
    case NSAPI_ERROR_OK:
      {
        ret = 1;
        break;
      }
  }

  if (ret == 1) {
    configureSocket(sock);
    _status = true;
  } else {
    _status = false;
  }

  return ret;
}

int arduino::MbedClient::connect(IPAddress ip, uint16_t port) {
  return connect(SocketHelpers::socketAddressFromIpAddress(ip, port));
}

int arduino::MbedClient::connect(const char *host, uint16_t port) {
  SocketAddress socketAddress = SocketAddress();
  socketAddress.set_port(port);
  SocketHelpers::gethostbyname(getNetwork(), host, &socketAddress);
  return connect(socketAddress);
}

int arduino::MbedClient::connectSSL(SocketAddress socketAddress) {
  // if a connection is aready ongoing, a disconnection must be enforced before starting another one
  stop();

  if (sock == nullptr) {
    sock = new TLSSocket();
    _own_socket = true;
  }
  if (sock == nullptr) {
    return 0;
  }

  if (beforeConnect) {
    beforeConnect();
  }

  if (static_cast<TLSSocket *>(sock)->open(getNetwork()) != NSAPI_ERROR_OK) {
    return 0;
  }

  /* For TLS connection timeout needs to be configured before handshake starts
   * otherwise socket timeout is not adopted. See TLSSocketWrapper::set_timeout(int timeout)
   */
  sock->set_timeout(_timeout);

restart_connect:
  nsapi_error_t returnCode = static_cast<TLSSocket *>(sock)->connect(socketAddress);
  int ret = 0;

  switch (returnCode) {
    case NSAPI_ERROR_IS_CONNECTED:
    case NSAPI_ERROR_OK:
      {
        ret = 1;
        break;
      }
    case NSAPI_ERROR_IN_PROGRESS:
    case NSAPI_ERROR_ALREADY:
      {
        delay(100);
        goto restart_connect;
      }
  }

  if (ret == 1) {
    configureSocket(sock);
    _status = true;
  } else {
    _status = false;
  }

  return ret;
}

int arduino::MbedClient::connectSSL(IPAddress ip, uint16_t port) {
  return connectSSL(SocketHelpers::socketAddressFromIpAddress(ip, port));
}

int arduino::MbedClient::connectSSL(const char *host, uint16_t port) {
  SocketAddress socketAddress = SocketAddress();
  socketAddress.set_port(port);
  SocketHelpers::gethostbyname(getNetwork(), host, &socketAddress);
  return connectSSL(socketAddress);
}

size_t arduino::MbedClient::write(uint8_t c) {
  return write(&c, 1);
}

size_t arduino::MbedClient::write(const uint8_t *buf, size_t size) {
  if (sock == nullptr)
    return 0;

  sock->set_timeout(_timeout);
  int ret = sock->send(buf, size);
  sock->set_blocking(false);
  return ret >= 0 ? ret : 0;
}

int arduino::MbedClient::available() {
  int ret = rxBuffer.available();
  return ret;
}

int arduino::MbedClient::read() {
  if (sock == nullptr)
    return -1;
  mutex->lock();
  if (!available()) {
    mutex->unlock();
    return -1;
  }

  int ret = rxBuffer.read_char();
  mutex->unlock();
  return ret;
}

int arduino::MbedClient::read(uint8_t *data, size_t len) {
  if (sock == nullptr)
    return 0;
  mutex->lock();
  int avail = available();

  if (!avail) {
    mutex->unlock();
    return -1;
  }

  if ((int)len > avail) {
    len = avail;
  }

  for (size_t i = 0; i < len; i++) {
    data[i] = rxBuffer.read_char();
  }
  mutex->unlock();

  return len;
}

int arduino::MbedClient::peek() {
  return rxBuffer.peek();
}

void arduino::MbedClient::flush() {
}

void arduino::MbedClient::stop() {
  if (mutex != nullptr) {
    mutex->lock();
  }
  if (sock != nullptr) {
    if (_own_socket) {
      delete sock;
    } else {
      sock->set_timeout(_timeout);
      sock->close();
    }
    sock = nullptr;
  }
  if (mutex != nullptr) {
    mutex->unlock();
  }
  if (reader_th != nullptr) {
    reader_th->join();
    delete reader_th;
    reader_th = nullptr;
  }
  if (event != nullptr) {
    delete event;
    event = nullptr;
  }
  if (mutex != nullptr) {
    delete mutex;
    mutex = nullptr;
  }
  _status = false;
  rxBuffer.clear();
}

uint8_t arduino::MbedClient::connected() {
  return ((status() == true) || (available() > 0));
}

IPAddress arduino::MbedClient::remoteIP() {
  return SocketHelpers::ipAddressFromSocketAddress(address);
}

uint16_t arduino::MbedClient::remotePort() {
  return address.get_port();
}

void arduino::MbedClient::setSocketTimeout(unsigned long timeout) {
  _timeout = timeout;
}
