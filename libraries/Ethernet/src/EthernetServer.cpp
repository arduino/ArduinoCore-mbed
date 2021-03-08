#include "EthernetServer.h"
#include "EthernetClient.h"

extern arduino::EthernetClass Ethernet;

arduino::EthernetServer::EthernetServer(uint16_t port) {
	_port = port;
}

uint8_t arduino::EthernetServer::status() {
	return 0;
}

void arduino::EthernetServer::begin() {
	if (sock == NULL) {
		sock = new TCPSocket();
		((TCPSocket*)sock)->open(Ethernet.getNetwork());
	}
	sock->bind(_port);
	sock->listen(5);
}

size_t arduino::EthernetServer::write(uint8_t c) {
	sock->send(&c, 1);
}

size_t arduino::EthernetServer::write(const uint8_t *buf, size_t size) {
	sock->send(buf, size);
}

arduino::EthernetClient arduino::EthernetServer::available(uint8_t* status) {
	EthernetClient client;
	nsapi_error_t error;
	TCPSocket* clientSocket = sock->accept(&error);
	if(status != nullptr) {
		*status = error == NSAPI_ERROR_OK ? 1 : 0;
	}
	client.setSocket(clientSocket);
	return client;
}
