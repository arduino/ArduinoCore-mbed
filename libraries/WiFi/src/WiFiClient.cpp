#include "WiFiClient.h"

extern WiFiClass WiFi;

#ifndef SOCKET_TIMEOUT
#define SOCKET_TIMEOUT 1500
#endif

arduino::WiFiClient::WiFiClient() {
}

uint8_t arduino::WiFiClient::status() {
	return _status;
}

void arduino::WiFiClient::receiveData() {
	uint8_t data[WIFI_RECEIVE_BUFFER_SIZE];
	_socket->set_blocking(false);
    int receivedBytes = _socket->recv(data, rxBuffer.availableForStore());

    for (int i = 0; i < receivedBytes; ++i) {
      rxBuffer.store_char(data[i]);
    }
    if (receivedBytes < 0 && receivedBytes != NSAPI_ERROR_WOULD_BLOCK) {
        _status = WL_CONNECTION_LOST;
    }
}

int arduino::WiFiClient::connect(SocketAddress socketAddress) {
	if (_socket == NULL) {
		_socket = new TCPSocket();		
		if(static_cast<TCPSocket*>(_socket)->open(WiFi.getNetwork()) != NSAPI_ERROR_OK){
			return 0;
		}
	}
	//sock->sigio(mbed::callback(this, &WiFiClient::getStatus));
	//sock->set_blocking(false);
	_socket->set_timeout(SOCKET_TIMEOUT);		
	nsapi_error_t returnCode = static_cast<TCPSocket*>(_socket)->connect(socketAddress);
	return returnCode == NSAPI_ERROR_OK ? 1 : 0;
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

int arduino::WiFiClient::connectSSL(SocketAddress socketAddress){
	if (_socket == NULL) {
		_socket = new TLSSocket();
		if(static_cast<TLSSocket*>(_socket)->open(WiFi.getNetwork()) != NSAPI_ERROR_OK){
			return 0;
		}
	}
	if (beforeConnect) {
		beforeConnect();
	}
	_socket->set_timeout(SOCKET_TIMEOUT);	
	nsapi_error_t returnCode = static_cast<TLSSocket*>(_socket)->connect(socketAddress);
	return returnCode == NSAPI_ERROR_OK ? 1 : 0;
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
	write(&c,1);
}

size_t arduino::WiFiClient::write(const uint8_t *buf, size_t size) {
	_socket->set_timeout(SOCKET_TIMEOUT);
	_socket->send(buf, size);
}

int arduino::WiFiClient::available() {
	if (rxBuffer.available() == 0) {
		receiveData();
	}
    return rxBuffer.available();
}

int arduino::WiFiClient::read() {
	if (!available()) {
    	return -1;
	}

	return rxBuffer.read_char();
}

int arduino::WiFiClient::read(uint8_t *data, size_t len) {
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

int arduino::WiFiClient::peek() {
	return rxBuffer.peek();
}

void arduino::WiFiClient::flush() {

}

void arduino::WiFiClient::stop() {
	if (_socket != NULL) {
		_socket->close();
		_socket = NULL;
	}
}

uint8_t arduino::WiFiClient::connected() {
	return _status != WL_CONNECTION_LOST;
}

IPAddress arduino::WiFiClient::remoteIP() {
	return IPAddress((uint32_t)0);
}

uint16_t arduino::WiFiClient::remotePort() {
	return 0;
}
