#include "EthernetSSLClient.h"

arduino::EthernetSSLClient::EthernetSSLClient(): _disableSNI{false}  {
  onBeforeConnect(mbed::callback(this, &EthernetSSLClient::setRootCA));
};
