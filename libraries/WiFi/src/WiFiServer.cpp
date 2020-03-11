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
	if (sock == NULL) {
		sock = new TCPSocket(WiFi.getNetwork());
	}
	sock->bind(_port);
	sock->listen(5);
}

size_t arduino::WiFiServer::write(uint8_t c) {
	sock->send(&c, 1);
}

size_t arduino::WiFiServer::write(const uint8_t *buf, size_t size) {
	sock->send(buf, size);
}

arduino::WiFiClient arduino::WiFiServer::available(uint8_t* status) {
	WiFiClient ret;
	TCPSocket* client = sock->accept();
	ret.setSocket(client);
	return ret;
}