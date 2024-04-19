#include "MbedSSLClient.h"

arduino::MbedSSLClient::MbedSSLClient(): _disableSNI{false} {
  onBeforeConnect(mbed::callback(this, &MbedSSLClient::setRootCA));
};
