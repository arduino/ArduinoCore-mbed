#include "MbedServer.h"
#include "MbedClient.h"

uint8_t arduino::MbedServer::status() {
  return 0;
}

void arduino::MbedServer::begin(uint16_t port) {
  _port = port;
  begin();
}

void arduino::MbedServer::begin() {
  if (sock == nullptr) {
    sock = new TCPSocket();
    ((TCPSocket *)sock)->open(getNetwork());
  }
  if (sock) {
    int enable = 1;
    sock->setsockopt(NSAPI_SOCKET, NSAPI_REUSEADDR, &enable, sizeof(int));
    sock->bind(_port);
    sock->listen(5);
    sock->set_blocking(false);
  }
}

// MUST be reimplemented (just copy/paste and replace MbedClient to *Client) since MbedClient is abstract

/*
arduino::MbedClient arduino::MbedServer::accept(uint8_t* status) {
	MbedClient client;
	nsapi_error_t error;
	if (sock == nullptr) {
		return client;
	}
	TCPSocket* clientSocket = sock->accept(&error);
	if(status != nullptr) {
		*status = error == NSAPI_ERROR_OK ? 1 : 0;
	}
	if (error == NSAPI_ERROR_OK) {
		client.setSocket(clientSocket);
	}
	return client;
}
*/
