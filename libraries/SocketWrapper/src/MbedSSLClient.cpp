#include "MbedSSLClient.h"

arduino::MbedSSLClient::MbedSSLClient() {
  onBeforeConnect(mbed::callback(this, &MbedSSLClient::setRootCA));
};