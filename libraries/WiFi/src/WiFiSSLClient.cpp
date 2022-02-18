#include "WiFiSSLClient.h"

arduino::WiFiSSLClient::WiFiSSLClient(): _disableSNI{false}  {
  onBeforeConnect(mbed::callback(this, &WiFiSSLClient::setRootCA));
};
