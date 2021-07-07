#include "EthernetClient.h"

#ifndef SOCKET_TIMEOUT
#define SOCKET_TIMEOUT 1000
#endif

arduino::EthernetClient::EthernetClient()
: _status(false)
{
}

uint8_t arduino::EthernetClient::status() {
	return _status;
}

void arduino::EthernetClient::setSocket(Socket* _sock) {
    sock = _sock;
    _status = true;
}

void arduino::EthernetClient::getStatus() {
	if (sock == nullptr) {
		_status = false;
		return;
	}

    uint8_t data[256];
    int ret = sock->recv(data, rxBuffer.availableForStore());
    for (int i = 0; i < ret; i++) {
      rxBuffer.store_char(data[i]);
    }
    if (ret < 0 && ret != NSAPI_ERROR_WOULD_BLOCK) {
        _status = false;
    }
}

int arduino::EthernetClient::connect(SocketAddress socketAddress) {
	if (sock == nullptr) {
		sock = new TCPSocket();		
		if(static_cast<TCPSocket*>(sock)->open(Ethernet.getNetwork()) != NSAPI_ERROR_OK){
			return 0;
		}
	}
	//sock->sigio(mbed::callback(this, &EthernetClient::getStatus));
	//sock->set_blocking(false);
	address = socketAddress;
	sock->set_timeout(SOCKET_TIMEOUT);		
	nsapi_error_t returnCode = static_cast<TCPSocket*>(sock)->connect(socketAddress);
	int ret = 0;
	switch (returnCode) {
	case NSAPI_ERROR_IS_CONNECTED:
	case NSAPI_ERROR_OK: {
		ret = 1;
		break;
	}
	}
	if (ret == 1)
		_status = true;
	return ret;
}

int arduino::EthernetClient::connect(IPAddress ip, uint16_t port) {	
	return connect(Ethernet.socketAddressFromIpAddress(ip, port));
}

int arduino::EthernetClient::connect(const char *host, uint16_t port) {
	SocketAddress socketAddress = SocketAddress();
	socketAddress.set_port(port);
	Ethernet.getNetwork()->gethostbyname(host, &socketAddress);
	return connect(socketAddress);
}

int arduino::EthernetClient::connectSSL(SocketAddress socketAddress){
	if (sock == nullptr) {
		sock = new TLSSocket();
		if(static_cast<TLSSocket*>(sock)->open(Ethernet.getNetwork()) != NSAPI_ERROR_OK){
			return 0;
		}
	}
	if (beforeConnect) {
		beforeConnect();
	}
	sock->set_timeout(SOCKET_TIMEOUT);	
	nsapi_error_t returnCode = static_cast<TLSSocket*>(sock)->connect(socketAddress);
	int ret = 0;
	switch (returnCode) {
	case NSAPI_ERROR_IS_CONNECTED:
	case NSAPI_ERROR_OK: {
		ret = 1;
		break;
	}
	}
	if (ret == 1)
		_status = true;

	return ret;
}

int arduino::EthernetClient::connectSSL(IPAddress ip, uint16_t port) {
	return connectSSL(Ethernet.socketAddressFromIpAddress(ip, port));
}

int arduino::EthernetClient::connectSSL(const char *host, uint16_t port) {
	SocketAddress socketAddress = SocketAddress();
	socketAddress.set_port(port);
	Ethernet.getNetwork()->gethostbyname(host, &socketAddress);
	return connectSSL(socketAddress);
}

size_t arduino::EthernetClient::write(uint8_t c) {
	if (sock == nullptr)
		return -1;
	auto ret = sock->send(&c, 1);
	return ret;
}

size_t arduino::EthernetClient::write(const uint8_t *buf, size_t size) {
	if (sock == nullptr)
		return -1;
	auto ret = sock->send(buf, size);
	return ret;
}

int arduino::EthernetClient::available() {
	if (rxBuffer.available() == 0) {
		getStatus();
	}
    return rxBuffer.available();
}

int arduino::EthernetClient::read() {
	if (!available()) {
    	return -1;
	}

	return rxBuffer.read_char();
}

int arduino::EthernetClient::read(uint8_t *data, size_t len) {
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

	return len;
}

int arduino::EthernetClient::peek() {
	return rxBuffer.peek();
}

void arduino::EthernetClient::flush() {

}

void arduino::EthernetClient::stop() {
	if (sock != nullptr) {
		sock->close();
		sock = nullptr;
	}
	_status = false;
}

uint8_t arduino::EthernetClient::connected() {
	getStatus();
	return _status;
}

IPAddress arduino::EthernetClient::remoteIP() {
	return Ethernet.ipAddressFromSocketAddress(address);
}

uint16_t arduino::EthernetClient::remotePort() {
	return 0;
}
