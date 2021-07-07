#include "WiFiServer.h"
#include "WiFiClient.h"

extern WiFiClass WiFi;

#ifndef WIFI_TCP_BUFFER_SIZE
#define WIFI_TCP_BUFFER_SIZE        1508
#endif

arduino::WiFiServer::WiFiServer(uint16_t port) {
	_port = port;
}

uint8_t arduino::WiFiServer::status() {
	return 0;
}

void arduino::WiFiServer::begin() {
	if (sock == nullptr) {
		sock = new TCPSocket();
		((TCPSocket*)sock)->open(WiFi.getNetwork());
	}
	if (sock) {
		sock->bind(_port);
		sock->listen(5);
	}
}

size_t arduino::WiFiServer::write(uint8_t c) {
	if (sock) {
		sock->send(&c, 1);
		return 1;
	}
	return 0;
}

size_t arduino::WiFiServer::write(const uint8_t *buf, size_t size) {
	if (sock) {
		sock->send(buf, size);
		return size;
	}
	return 0;
}

arduino::WiFiClient arduino::WiFiServer::available(uint8_t* status) {
	WiFiClient client;
	nsapi_error_t error;
	if (sock == nullptr) {
		return client;
	}
	TCPSocket* clientSocket = sock->accept(&error);
	if(status != nullptr) {
		*status = error == NSAPI_ERROR_OK ? 1 : 0;
	}
	client.setSocket(clientSocket);
	return client;
}
