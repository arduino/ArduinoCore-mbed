#include "WiFiClient.h"

extern WiFiClass WiFi;

#ifndef SOCKET_TIMEOUT
#define SOCKET_TIMEOUT 1500
#endif

arduino::WiFiClient::WiFiClient():
    _status(false)
{
}

uint8_t arduino::WiFiClient::status() {
	return _status;
}

void arduino::WiFiClient::readSocket() {
	while (1) {
		event->wait_any(0xFF, 100);
	    uint8_t data[SOCKET_BUFFER_SIZE];
	    int ret = NSAPI_ERROR_WOULD_BLOCK;
	    do {
	    	mutex->lock();
	    	if (rxBuffer.availableForStore() == 0) {
	    		yield();
	    	}
	    	if (sock == nullptr || (closing && borrowed_socket)) {
	    		goto cleanup;
			}
		    ret = sock->recv(data, rxBuffer.availableForStore());
		    if (ret < 0 && ret != NSAPI_ERROR_WOULD_BLOCK) {
		    	goto cleanup;
		    }
		    for (int i = 0; i < ret; i++) {
		      rxBuffer.store_char(data[i]);
		    }
		   	_status = true;
		    mutex->unlock();
		} while (ret == NSAPI_ERROR_WOULD_BLOCK || ret > 0);
	}
cleanup:
	_status = false;
	mutex->unlock();
	return;
}

void arduino::WiFiClient::getStatus() {
	event->set(1);
}

void arduino::WiFiClient::setSocket(Socket* _sock) {
  sock = _sock;
  configureSocket(sock);
}

void arduino::WiFiClient::configureSocket(Socket* _s) {
	_s->set_timeout(0);
	_s->set_blocking(false);

	if (event == nullptr) {
		event = new rtos::EventFlags;
	}
	if (mutex == nullptr) {
		mutex = new rtos::Mutex;
	}
	mutex->lock();
	if (reader_th == nullptr) {
		reader_th = new rtos::Thread;
		reader_th->start(mbed::callback(this, &WiFiClient::readSocket));
	}
	mutex->unlock();
	_s->sigio(mbed::callback(this, &WiFiClient::getStatus));
	_status = true;
}

int arduino::WiFiClient::connect(SocketAddress socketAddress) {
	if (sock == nullptr) {
		sock = new TCPSocket();
		_own_socket = true;
	}
	if (sock == nullptr) {
		return 0;
	}

	if(static_cast<TCPSocket*>(sock)->open(WiFi.getNetwork()) != NSAPI_ERROR_OK){
		return 0;
	}

	address = socketAddress;
	nsapi_error_t returnCode = static_cast<TCPSocket*>(sock)->connect(socketAddress);
	int ret = 0;

	switch (returnCode) {
	case NSAPI_ERROR_IS_CONNECTED:
	case NSAPI_ERROR_OK: {
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

int arduino::WiFiClient::connect(IPAddress ip, uint16_t port) {	
	return connect(WiFi.socketAddressFromIpAddress(ip, port));
}

int arduino::WiFiClient::connect(const char *host, uint16_t port) {
	SocketAddress socketAddress = SocketAddress();
	socketAddress.set_port(port);
	WiFi.getNetwork()->gethostbyname(host, &socketAddress);
	return connect(socketAddress);
}

int arduino::WiFiClient::connectSSL(SocketAddress socketAddress) {
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

	if(static_cast<TLSSocket*>(sock)->open(WiFi.getNetwork()) != NSAPI_ERROR_OK){
		return 0;
	}

	address = socketAddress;

restart_connect:
	nsapi_error_t returnCode = static_cast<TLSSocket*>(sock)->connect(socketAddress);
	int ret = 0;

	switch (returnCode) {
		case NSAPI_ERROR_IS_CONNECTED:
		case NSAPI_ERROR_OK: {
			ret = 1;
			break;
		}
		case NSAPI_ERROR_IN_PROGRESS:
		case NSAPI_ERROR_ALREADY: {
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

int arduino::WiFiClient::connectSSL(IPAddress ip, uint16_t port) {
	return connectSSL(WiFi.socketAddressFromIpAddress(ip, port));
}

int arduino::WiFiClient::connectSSL(const char *host, uint16_t port) {
	SocketAddress socketAddress = SocketAddress();
	socketAddress.set_port(port);
	WiFi.getNetwork()->gethostbyname(host, &socketAddress);
	return connectSSL(socketAddress);
}

size_t arduino::WiFiClient::write(uint8_t c) {
	return write(&c, 1);
}

size_t arduino::WiFiClient::write(const uint8_t *buf, size_t size) {
	if (sock == nullptr)
		return 0;

	sock->set_blocking(true);
	sock->set_timeout(SOCKET_TIMEOUT);
	sock->send(buf, size);
	configureSocket(sock);
	return size;
}

int arduino::WiFiClient::available() {
	int ret = rxBuffer.available();
    return ret;
}

int arduino::WiFiClient::read() {
	mutex->lock();
	if (!available()) {
    	return -1;
	}

	int ret = rxBuffer.read_char();
	mutex->unlock();
	return ret;
}

int arduino::WiFiClient::read(uint8_t *data, size_t len) {
	mutex->lock();
	int avail = available();

	if (!avail) {
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

int arduino::WiFiClient::peek() {
	return rxBuffer.peek();
}

void arduino::WiFiClient::flush() {

}

void arduino::WiFiClient::stop() {
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

uint8_t arduino::WiFiClient::connected() {
	return _status;
}

IPAddress arduino::WiFiClient::remoteIP() {
	return WiFi.ipAddressFromSocketAddress(address);
}

uint16_t arduino::WiFiClient::remotePort() {
	return 0;
}
