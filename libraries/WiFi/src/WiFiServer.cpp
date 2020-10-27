#include "WiFiServer.h"
#include "WiFiClient.h"

extern WiFiClass WiFi;

#ifndef WIFI_TCP_BUFFER_SIZE
#define WIFI_TCP_BUFFER_SIZE        1508
#endif

#define MAX_PENDING_CONNECTIONS 5

arduino::WiFiServer::WiFiServer(uint16_t port) {
	_port = port;
	_socketState = SOCK_CLOSED;
}

uint8_t arduino::WiFiServer::status() {
	return _socketState;
}

void arduino::WiFiServer::begin() {
	if (_socket == NULL) {
		_socket = new TCPSocket();
		((TCPSocket*)_socket)->open(WiFi.getNetwork());
	}
	_socket->bind(_port);
	_socket->listen(MAX_PENDING_CONNECTIONS);
	_socketState = SOCK_LISTEN;
}

	}
}

size_t arduino::WiFiServer::write(uint8_t c) {
	write(&c, 1);	
}

size_t arduino::WiFiServer::write(const uint8_t *buf, size_t size) {
	_socket->set_blocking(true);
	_socket->send(buf, size);
}

arduino::WiFiClient arduino::WiFiServer::available(uint8_t* status) {
	WiFiClient client;
	nsapi_error_t error;
	_socket->set_blocking(false);
	TCPSocket* clientSocket = _socket->accept(&error);
	
	if(status != nullptr) {
		*status = error == NSAPI_ERROR_OK ? 1 : 0;
	}

	if(error == NSAPI_ERROR_OK){
		client.setSocket(clientSocket);
	}
	
	return client;
}
