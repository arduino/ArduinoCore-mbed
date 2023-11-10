#include "SocketHelpers.h"

uint8_t* arduino::MbedSocketClass::macAddress(uint8_t* mac) {
  const char* mac_str = getNetwork()->get_mac_address();
  for (int b = 0; b < 6; b++) {
    uint32_t tmp;
    sscanf(&mac_str[b * 2 + (b)], "%02x", (unsigned int*)&tmp);
    mac[5 - b] = (uint8_t)tmp;
  }
  //sscanf(mac_str, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx", &mac[5], &mac[4], &mac[3], &mac[2], &mac[1], &mac[0]);
  return mac;
}

int arduino::MbedSocketClass::hostByName(const char* aHostname, IPAddress& aResult) {
  SocketAddress socketAddress = SocketAddress();
  nsapi_error_t returnCode = getNetwork()->gethostbyname(aHostname, &socketAddress);
  nsapi_addr_t address = socketAddress.get_addr();
  aResult[0] = address.bytes[0];
  aResult[1] = address.bytes[1];
  aResult[2] = address.bytes[2];
  aResult[3] = address.bytes[3];
  return returnCode == NSAPI_ERROR_OK ? 1 : 0;
}

arduino::IPAddress arduino::MbedSocketClass::localIP() {
  SocketAddress ip;
  NetworkInterface* interface = getNetwork();
  interface->get_ip_address(&ip);
  return ipAddressFromSocketAddress(ip);
}

arduino::IPAddress arduino::MbedSocketClass::subnetMask() {
  SocketAddress ip;
  NetworkInterface* interface = getNetwork();
  interface->get_netmask(&ip);
  return ipAddressFromSocketAddress(ip);
}

arduino::IPAddress arduino::MbedSocketClass::gatewayIP() {
  SocketAddress ip;
  NetworkInterface* interface = getNetwork();
  interface->get_gateway(&ip);
  return ipAddressFromSocketAddress(ip);
}

arduino::IPAddress arduino::MbedSocketClass::dnsServerIP() {
  SocketAddress ip;
  NetworkInterface* interface = getNetwork();
  interface->get_dns_server(0, &ip, nullptr);
  return ipAddressFromSocketAddress(ip);
}

arduino::IPAddress arduino::MbedSocketClass::dnsIP(int n) {
  SocketAddress ip;
  NetworkInterface* interface = getNetwork();
  interface->get_dns_server(n, &ip, nullptr);
  return ipAddressFromSocketAddress(ip);
}

void arduino::MbedSocketClass::config(arduino::IPAddress local_ip) {
  nsapi_addr_t convertedIP = { NSAPI_IPv4, { local_ip[0], local_ip[1], local_ip[2], local_ip[3] } };
  _ip = SocketAddress(convertedIP);
}

void arduino::MbedSocketClass::config(const char* local_ip) {
  _ip = SocketAddress(local_ip);
}

void arduino::MbedSocketClass::config(IPAddress local_ip, IPAddress dns_server) {
  config(local_ip);
  setDNS(dns_server);
}

void arduino::MbedSocketClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway) {
  config(local_ip, dns_server);
  nsapi_addr_t convertedGatewayIP = { NSAPI_IPv4, { gateway[0], gateway[1], gateway[2], gateway[3] } };
  _gateway = SocketAddress(convertedGatewayIP);
}

void arduino::MbedSocketClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet) {
  config(local_ip, dns_server, gateway);
  nsapi_addr_t convertedSubnetMask = { NSAPI_IPv4, { subnet[0], subnet[1], subnet[2], subnet[3] } };
  _netmask = SocketAddress(convertedSubnetMask);
}

void arduino::MbedSocketClass::setDNS(IPAddress dns_server1) {
  nsapi_addr_t convertedDNSServer = { NSAPI_IPv4, { dns_server1[0], dns_server1[1], dns_server1[2], dns_server1[3] } };
  _dnsServer1 = SocketAddress(convertedDNSServer);
}

void arduino::MbedSocketClass::setDNS(IPAddress dns_server1, IPAddress dns_server2) {
  setDNS(dns_server1);
  nsapi_addr_t convertedDNSServer2 = { NSAPI_IPv4, { dns_server2[0], dns_server2[1], dns_server2[2], dns_server2[3] } };
  _dnsServer2 = SocketAddress(convertedDNSServer2);
}

arduino::IPAddress arduino::MbedSocketClass::ipAddressFromSocketAddress(SocketAddress socketAddress) {
  nsapi_addr_t address = socketAddress.get_addr();
  return IPAddress(address.bytes[0], address.bytes[1], address.bytes[2], address.bytes[3]);
}

SocketAddress arduino::MbedSocketClass::socketAddressFromIpAddress(arduino::IPAddress ip, uint16_t port) {
  nsapi_addr_t convertedIP = { NSAPI_IPv4, { ip[0], ip[1], ip[2], ip[3] } };
  return SocketAddress(convertedIP, port);
}


// Download helper

#include "utility/http_request.h"
#include "utility/https_request.h"

void MbedSocketClass::setFeedWatchdogFunc(voidFuncPtr func) {
  _feed_watchdog_func = func;
}

void MbedSocketClass::feedWatchdog() {
  if (_feed_watchdog_func)
    _feed_watchdog_func();
}

void MbedSocketClass::body_callback(const char* data, uint32_t data_len) {
  feedWatchdog();
  fwrite(data, 1, data_len, download_target);
}

int MbedSocketClass::download(char* url, const char* target_file, bool const is_https) {
  download_target = fopen(target_file, "wb");

  HttpRequest* req_http = nullptr;
  HttpsRequest* req_https = nullptr;
  HttpResponse* rsp = nullptr;

  if (is_https) {
    req_https = new HttpsRequest(getNetwork(), nullptr, HTTP_GET, url, mbed::callback(this, &MbedSocketClass::body_callback));
    rsp = req_https->send(NULL, 0);
    if (rsp == NULL) {
      fclose(download_target);
      return req_https->get_error();
    }
  } else {
    req_http = new HttpRequest(getNetwork(), HTTP_GET, url, mbed::callback(this, &MbedSocketClass::body_callback));
    rsp = req_http->send(NULL, 0);
    if (rsp == NULL) {
      fclose(download_target);
      return req_http->get_error();
    }
  }

  while (!rsp->is_message_complete()) {
    delay(10);
  }

  int const size = ftell(download_target);
  fclose(download_target);
  return size;
}
