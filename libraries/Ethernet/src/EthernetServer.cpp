#include "EthernetServer.h"

arduino::EthernetClient arduino::EthernetServer::available(uint8_t* status) {
  return accept(status);
}

arduino::EthernetClient arduino::EthernetServer::accept(uint8_t* status) {
  EthernetClient client;
  nsapi_error_t error;

  if (sock == nullptr) {
    return client;
  }
  TCPSocket* clientSocket = sock->accept(&error);
  if (status != nullptr) {
    *status = error == NSAPI_ERROR_OK ? 1 : 0;
  }
  if (error == NSAPI_ERROR_OK) {
    client.setSocket(clientSocket);
  }
  return client;
}
