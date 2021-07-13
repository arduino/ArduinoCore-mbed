#include "MbedServer.h"
#include "MbedClient.h"

uint8_t arduino::MbedServer::status() {
  return 0;
}

void arduino::MbedServer::begin() {
  if (sock == nullptr) {
    sock = new TCPSocket();
    ((TCPSocket *)sock)->open(getNetwork());
  }
  if (sock) {
    sock->bind(_port);
    sock->listen(5);
    sock->set_blocking(false);
  }
}

size_t arduino::MbedServer::write(uint8_t c) {
  if (sock) {
    sock->send(&c, 1);
    return 1;
  }
  return 0;
}

size_t arduino::MbedServer::write(const uint8_t *buf, size_t size) {
  if (sock) {
    sock->send(buf, size);
    return size;
  }
  return 0;
}


// MUST be reimplemented (just copy/paste and replace MbedClient to *Client) since MbedClient is abstract

/*
arduino::MbedClient arduino::MbedServer::available(uint8_t* status) {
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
