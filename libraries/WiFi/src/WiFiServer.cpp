#include "WiFiServer.h"

arduino::WiFiClient arduino::WiFiServer::available(uint8_t* status) {
  return accept(status);
}

arduino::WiFiClient arduino::WiFiServer::accept(uint8_t* status) {
  WiFiClient client;
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