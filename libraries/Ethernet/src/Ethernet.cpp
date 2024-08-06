#include "Ethernet.h"

int arduino::EthernetClass::begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout) {
  if (eth_if == nullptr) {
    return 0;
  }
  eth_if->set_dhcp(true);
  return _begin(mac, timeout, responseTimeout);
}

int arduino::EthernetClass::_begin(uint8_t *mac, unsigned long timeout, unsigned long responseTimeout) {
  if (mac != nullptr) {
    eth_if->get_emac().set_hwaddr(mac);
  }

  unsigned long start = millis();
  eth_if->set_blocking(false);
  eth_if->connect();

  while ((millis() - start < timeout) && (linkStatus() != LinkON)) {
    delay(10);
  }

  return (linkStatus() == LinkON ? 1 : 0);
}

int arduino::EthernetClass::begin(uint8_t *mac, IPAddress ip) {
  IPAddress dns = ip;
  dns[3] = 1;

  auto ret = begin(mac, ip, dns);
  return ret;
}

int arduino::EthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns) {
  IPAddress gateway = ip;
  gateway[3] = 1;

  auto ret = begin(mac, ip, dns, gateway);
  return ret;
}

int arduino::EthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway) {
  IPAddress subnet(255, 255, 255, 0);
  auto ret = begin(mac, ip, dns, gateway, subnet);
  return ret;
}

int arduino::EthernetClass::begin(uint8_t *mac, IPAddress ip, IPAddress dns, IPAddress gateway, IPAddress subnet, unsigned long timeout, unsigned long responseTimeout) {
  if(eth_if == nullptr) {
    return 0;
  }

  config(ip, dns, gateway, subnet);

  eth_if->set_dhcp(false);
  eth_if->set_network(_ip, _netmask, _gateway);

  auto ret = _begin(mac, timeout, responseTimeout);

  char if_name[5];
  eth_if->get_interface_name(if_name);
  eth_if->add_dns_server(_dnsServer1, if_name);

  return ret;
}

void arduino::EthernetClass::end() {
  disconnect();
}

EthernetLinkStatus arduino::EthernetClass::linkStatus() {
  if(eth_if == nullptr) {
    return LinkOFF;
  }
  return (eth_if->get_connection_status() == NSAPI_STATUS_GLOBAL_UP ? LinkON : LinkOFF);
}

EthernetHardwareStatus arduino::EthernetClass::hardwareStatus() {
  return EthernetMbed;
}


int arduino::EthernetClass::disconnect() {
  if(eth_if != nullptr) {
    eth_if->disconnect();
  }
  return 1;
}


uint8_t arduino::EthernetClass::status() {
  return _currentNetworkStatus;
}

NetworkInterface *arduino::EthernetClass::getNetwork() {
  return eth_if;
}

unsigned long arduino::EthernetClass::getTime() {
  return 0;
}

void arduino::EthernetClass::MACAddress(uint8_t *mac_address)
{
  macAddress(mac_address);
}

arduino::EthernetClass Ethernet(static_cast<EthernetInterface*>(EthInterface::get_default_instance()));
