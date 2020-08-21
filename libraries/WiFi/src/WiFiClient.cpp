#include "WiFiClient.h"

extern WiFiClass WiFi;

#ifndef WIFI_TCP_BUFFER_SIZE
#define WIFI_TCP_BUFFER_SIZE        1508
#endif

#ifndef SOCKET_TIMEOUT
#define SOCKET_TIMEOUT 1000
#endif

arduino::WiFiClient::WiFiClient() {
}

uint8_t arduino::WiFiClient::status() {
	return _status;
}

void arduino::WiFiClient::getStatus() {
    uint8_t data[256];
    int ret = sock->recv(data, rxBuffer.availableForStore());
    for (int i = 0; i < ret; i++) {
      rxBuffer.store_char(data[i]);
    }
    if (ret < 0 && ret != NSAPI_ERROR_WOULD_BLOCK) {
        _status = WL_CONNECTION_LOST;
    }
}

int arduino::WiFiClient::connect(SocketAddress socketAddress) {
	if (sock == NULL) {
		sock = new TCPSocket();		
		if(static_cast<TCPSocket*>(sock)->open(WiFi.getNetwork()) != NSAPI_ERROR_OK){
			return 0;
		}
	}
	//sock->sigio(mbed::callback(this, &WiFiClient::getStatus));
	//sock->set_blocking(false);
	sock->set_timeout(SOCKET_TIMEOUT);		
	nsapi_error_t returnCode = static_cast<TCPSocket*>(sock)->connect(socketAddress);
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
	if (sock == NULL) {
		sock = new TLSSocket();
		if(static_cast<TLSSocket*>(sock)->open(WiFi.getNetwork()) != NSAPI_ERROR_OK){
			return 0;
		}
	}
	if (beforeConnect) {
		beforeConnect();
	}
	sock->set_timeout(SOCKET_TIMEOUT);	
	nsapi_error_t returnCode = static_cast<TLSSocket*>(sock)->connect(socketAddress);
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
	sock->send(&c, 1);
}

size_t arduino::WiFiClient::write(const uint8_t *buf, size_t size) {
	sock->send(buf, size);
}

int arduino::WiFiClient::available() {
	if (rxBuffer.available() == 0) {
		getStatus();
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
	if (sock != NULL) {
		sock->close();
		sock = NULL;
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
