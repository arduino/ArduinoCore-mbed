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
		sock = new TCPSocket();
		((TCPSocket*)sock)->open(WiFi.getNetwork());
	}
	sock->bind(_port);
	sock->listen(5);
}

size_t arduino::WiFiServer::write(uint8_t c) {
	write(&c, 1);	
}

size_t arduino::WiFiServer::write(const uint8_t *buf, size_t size) {
	sock->send(buf, size);
}

arduino::WiFiClient arduino::WiFiServer::available(uint8_t* status) {
	WiFiClient client;
	nsapi_error_t error;
	sock->set_blocking(false);
	TCPSocket* clientSocket = sock->accept(&error);
	
	if(status != nullptr) {
		*status = error == NSAPI_ERROR_OK ? 1 : 0;
	}

	if(error == NSAPI_ERROR_OK){
		client.setSocket(clientSocket);		
	}

	sock->set_blocking(true);
	return client;
}
