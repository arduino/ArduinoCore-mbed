#include "MbedClient.h"

#ifndef SOCKET_TIMEOUT
#define SOCKET_TIMEOUT 1500
#endif

arduino::MbedClient::MbedClient()
  : _status(false),
    _timeout(0) {
}

uint8_t arduino::MbedClient::status() {
  return _status;
}

void arduino::MbedClient::readSocket() {
  while (1) {
    event->wait_any(0xFF, 100);
    uint8_t data[SOCKET_BUFFER_SIZE];
    int ret = NSAPI_ERROR_WOULD_BLOCK;
    do {
      if (rxBuffer.availableForStore() == 0) {
        yield();
        continue;
      }
      mutex->lock();
      if (sock == nullptr || (closing && borrowed_socket)) {
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
  _s->set_timeout(_timeout);
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
    reader_th = new rtos::Thread(osPriorityNormal - 2);
    reader_th->start(mbed::callback(this, &MbedClient::readSocket));
  }
  mutex->unlock();
  _s->sigio(mbed::callback(this, &MbedClient::getStatus));
  _status = true;
}

int arduino::MbedClient::connect(SocketAddress socketAddress) {

  if (sock && reader_th) {
    // trying to reuse a connection, let's call stop() to cleanup the state
    char c;
    if (sock->recv(&c, 1) < 0) {
      stop();
    }
  }

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
  getNetwork()->gethostbyname(host, &socketAddress);
  return connect(socketAddress);
}

int arduino::MbedClient::connectSSL(SocketAddress socketAddress) {
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

int arduino::MbedClient::connectSSL(const char *host, uint16_t port, bool disableSNI) {
  if (!disableSNI) {
    if (sock == nullptr) {
      sock = new TLSSocket();
      _own_socket = true;
    }
    static_cast<TLSSocket *>(sock)->set_hostname(host);
  }

  SocketAddress socketAddress = SocketAddress();
  socketAddress.set_port(port);
  getNetwork()->gethostbyname(host, &socketAddress);
  return connectSSL(socketAddress);
}

size_t arduino::MbedClient::write(uint8_t c) {
  return write(&c, 1);
}

size_t arduino::MbedClient::write(const uint8_t *buf, size_t size) {
  if (sock == nullptr)
    return 0;

  sock->set_blocking(true);
  sock->set_timeout(SOCKET_TIMEOUT);
  int ret = NSAPI_ERROR_WOULD_BLOCK;
  do {
    ret = sock->send(buf, size);
  } while ((ret != size && ret == NSAPI_ERROR_WOULD_BLOCK) && connected());
  configureSocket(sock);
  return size;
}

int arduino::MbedClient::available() {
  int ret = rxBuffer.available();
  return ret;
}

int arduino::MbedClient::read() {
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
  if (sock != nullptr && borrowed_socket == false) {
    if (_own_socket) {
      delete sock;
    } else {
      sock->close();
    }
    sock = nullptr;
  }
  closing = true;
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
}

uint8_t arduino::MbedClient::connected() {
  return _status;
}

IPAddress arduino::MbedClient::remoteIP() {
  return SocketHelpers::ipAddressFromSocketAddress(address);
}

uint16_t arduino::MbedClient::remotePort() {
  return address.get_port();
}

void arduino::MbedClient::setTimeout(unsigned long timeout) {
  _timeout = timeout;
}
