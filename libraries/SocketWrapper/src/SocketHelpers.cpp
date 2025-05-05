#include "SocketHelpers.h"
#include <ICMPSocket.h>

uint8_t* arduino::MbedSocketClass::macAddress(uint8_t* mac) {
  const char* mac_str = getNetwork()->get_mac_address();
  for (int b = 0; b < 6; b++) {
    uint32_t tmp;
    sscanf(&mac_str[b * 2 + (b)], "%02x", (unsigned int*)&tmp);
    mac[b] = (uint8_t)tmp;
  }
  return mac;
}

String arduino::MbedSocketClass::macAddress() {
  const char* mac_str = getNetwork()->get_mac_address();
  if (!mac_str) {
    return String("ff:ff:ff:ff:ff:ff");
  }
  return String(mac_str);
}

int arduino::MbedSocketClass::setHostname(const char* hostname) {
  NetworkInterface* interface = getNetwork();
  interface->set_hostname(hostname);
  return 1;
}

int arduino::MbedSocketClass::hostByName(const char* aHostname, IPAddress& aResult) {
  SocketAddress socketAddress = SocketAddress();
  nsapi_error_t returnCode = gethostbyname(getNetwork(), aHostname, &socketAddress);
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
  char _if_name[5] {};
  interface->get_interface_name(_if_name);
  interface->get_dns_server(0, &ip, _if_name);
  return ipAddressFromSocketAddress(ip);
}

arduino::IPAddress arduino::MbedSocketClass::dnsIP(int n) {
  SocketAddress ip;
  NetworkInterface* interface = getNetwork();
  char _if_name[5] {};
  interface->get_interface_name(_if_name);
  interface->get_dns_server(n, &ip, _if_name);
  return ipAddressFromSocketAddress(ip);
}

int arduino::MbedSocketClass::ping(const char *hostname, uint8_t ttl, uint32_t timeout)
{
  SocketAddress socketAddress;
  gethostbyname(getNetwork(),hostname, &socketAddress);
  return ping(socketAddress, ttl, timeout);
}

int arduino::MbedSocketClass::ping(const String &hostname, uint8_t ttl, uint32_t timeout)
{
  return ping(hostname.c_str(), ttl, timeout);
}

int arduino::MbedSocketClass::ping(IPAddress host, uint8_t ttl, uint32_t timeout)
{
  SocketAddress socketAddress = socketAddressFromIpAddress(host, 0);
  return ping(socketAddress, ttl, timeout);
}

void arduino::MbedSocketClass::config(arduino::IPAddress local_ip) {
  IPAddress dns = local_ip;
  dns[3] = 1;
  config(local_ip, dns);
}

void arduino::MbedSocketClass::config(const char* local_ip) {
  _ip = SocketAddress(local_ip);
}

void arduino::MbedSocketClass::config(IPAddress local_ip, IPAddress dns_server) {
  IPAddress gw = local_ip;
  gw[3] = 1;
  config(local_ip, dns_server, gw);
}

void arduino::MbedSocketClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway) {
  IPAddress nm(255, 255, 255, 0);
  config(local_ip, dns_server, gateway, nm);
}

void arduino::MbedSocketClass::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet) {
  _useStaticIP = (local_ip != INADDR_NONE);
  if (!_useStaticIP)
    return;
  nsapi_addr_t convertedIP = { NSAPI_IPv4, { local_ip[0], local_ip[1], local_ip[2], local_ip[3] } };
  _ip = SocketAddress(convertedIP);
  nsapi_addr_t convertedGatewayIP = { NSAPI_IPv4, { gateway[0], gateway[1], gateway[2], gateway[3] } };
  _gateway = SocketAddress(convertedGatewayIP);
  nsapi_addr_t convertedSubnetMask = { NSAPI_IPv4, { subnet[0], subnet[1], subnet[2], subnet[3] } };
  _netmask = SocketAddress(convertedSubnetMask);
  setDNS(dns_server);
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

int arduino::MbedSocketClass::ping(SocketAddress &socketAddress, uint8_t ttl, uint32_t timeout)
{
  /* ttl is not supported by mbed ICMPSocket. Default value used is 255 */
  (void)ttl;
  int response = -1;
#if MBED_CONF_LWIP_RAW_SOCKET_ENABLED
  ICMPSocket s;
  s.set_timeout(timeout);
  s.open(getNetwork());
  response = s.ping(socketAddress, timeout);
  s.close();
#endif

  return response;
}

arduino::IPAddress arduino::MbedSocketClass::ipAddressFromSocketAddress(SocketAddress socketAddress) {
  nsapi_addr_t address = socketAddress.get_addr();
  return IPAddress(address.bytes[0], address.bytes[1], address.bytes[2], address.bytes[3]);
}

SocketAddress arduino::MbedSocketClass::socketAddressFromIpAddress(arduino::IPAddress ip, uint16_t port) {
  nsapi_addr_t convertedIP = { NSAPI_IPv4, { ip[0], ip[1], ip[2], ip[3] } };
  return SocketAddress(convertedIP, port);
}

nsapi_error_t arduino::MbedSocketClass::gethostbyname(NetworkInterface* interface, const char* aHostname, SocketAddress* socketAddress) {
  char ifname[5] {};
  interface->get_interface_name(ifname);
  nsapi_version_t version = NSAPI_IPv4;
#if MBED_CONF_LWIP_IPV6_ENABLED
  version = NSAPI_UNSPEC;
#endif
  return interface->gethostbyname(aHostname, socketAddress, version, ifname);
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
  fwrite(data, sizeof(data[0]), data_len, download_target);
}

int MbedSocketClass::download(const char* url, const char* target_file, bool const is_https) {
  download_target = fopen(target_file, "wb");

  int res = this->download(url, is_https, mbed::callback(this, &MbedSocketClass::body_callback));

  fclose(download_target);
  download_target = nullptr;

  return res;
}

int MbedSocketClass::download(const char* url, bool const is_https, mbed::Callback<void(const char*, uint32_t)> cbk) {
  if(cbk == nullptr) {
    return 0; // a call back must be set
  }

  HttpRequest* req_http = nullptr;
  HttpsRequest* req_https = nullptr;
  HttpResponse* rsp = nullptr;
  int res=0;
  std::vector<string*> header_fields;

  if (is_https) {
    req_https = new HttpsRequest(getNetwork(), nullptr, HTTP_GET, url, cbk);
    rsp = req_https->send(NULL, 0);
    if (rsp == NULL) {
      res = req_https->get_error();
      goto exit;
    }
  } else {
    req_http = new HttpRequest(getNetwork(), HTTP_GET, url, cbk);
    rsp = req_http->send(NULL, 0);
    if (rsp == NULL) {
      res = req_http->get_error();
      goto exit;
    }
  }

  while (!rsp->is_message_complete()) {
    delay(10);
  }

  // find the header containing the "Content-Length" value and return that
  header_fields = rsp->get_headers_fields();
  for(int i=0; i<header_fields.size(); i++) {

    if(strcmp(header_fields[i]->c_str(), "Content-Length") == 0) {
      res = std::stoi(*rsp->get_headers_values()[i]);
      break;
    }
  }

exit:
  if(req_http)  delete req_http;
  if(req_https) delete req_https;
  // no need to delete rsp, it is already deleted by deleting the request
  // this may be harmful since it can allow dangling pointers existence

  return res;
}
