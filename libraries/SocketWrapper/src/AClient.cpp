
#include "AClient.h"
#include "MbedSSLClient.h"

void arduino::AClient::newMbedClient() {
  client.reset(new MbedClient());
  client->setNetwork(getNetwork());
}

arduino::AClient::operator bool() {
  return client && *client;
}

void arduino::AClient::setSocket(Socket *sock) {
  if (!client) {
    newMbedClient();
  }
  client->setSocket(sock);
}

void arduino::AClient::setSocketTimeout(unsigned long timeout) {
  if (!client) {
    newMbedClient();
  }
  client->setSocketTimeout(timeout);
}

int arduino::AClient::connect(IPAddress ip, uint16_t port) {
  if (!client) {
    newMbedClient();
  }
  return client->connect(ip, port);
}

int arduino::AClient::connect(const char *host, uint16_t port) {
  if (!client) {
    newMbedClient();
  }
  return client->connect(host, port);
}

int arduino::AClient::connectSSL(IPAddress ip, uint16_t port) {
  if (!client) {
    newMbedClient();
  }
  return client->connectSSL(ip, port);
}

int arduino::AClient::connectSSL(const char *host, uint16_t port) {
  if (!client) {
    newMbedClient();
  }
  return client->connectSSL(host, port);
}

void arduino::AClient::stop() {
  if (!client)
    return;
  client->stop();
}

uint8_t arduino::AClient::connected() {
  if (!client)
    return false;
  return client->connected();
}

uint8_t arduino::AClient::status() {
  if (!client)
    return false;
  return client->status();
}

IPAddress arduino::AClient::remoteIP() {
  if (!client)
    return INADDR_NONE;
  return client->remoteIP();
}

uint16_t arduino::AClient::remotePort() {
  if (!client)
    return 0;
  return client->remotePort();
}

size_t arduino::AClient::write(uint8_t b) {
  if (!client)
    return 0;
  return client->write(b);
}

size_t arduino::AClient::write(const uint8_t *buf, size_t size) {
  if (!client)
    return 0;
  return client->write(buf, size);
}

void arduino::AClient::flush() {
  if (!client)
    return;
  client->flush();
}

int arduino::AClient::available() {
  if (!client)
    return 0;
  return client->available();
}

int arduino::AClient::read() {
  if (!client)
    return -1;
  return client->read();
}

int arduino::AClient::read(uint8_t *buf, size_t size) {
  if (!client)
    return 0;
  return client->read(buf, size);
}

int arduino::AClient::peek() {
  if (!client)
    return -1;
  return client->peek();
}

void arduino::ASslClient::newMbedClient() {
  client.reset(new MbedSSLClient());
  client->setNetwork(getNetwork());
}

void arduino::ASslClient::disableSNI(bool statusSNI) {
  if (!client) {
    newMbedClient();
  }
  static_cast<MbedSSLClient*>(client.get())->disableSNI(statusSNI);
}

void arduino::ASslClient::appendCustomCACert(const char* ca_cert) {
  if (!client) {
    newMbedClient();
  }
  static_cast<MbedSSLClient*>(client.get())->appendCustomCACert(ca_cert);
}
