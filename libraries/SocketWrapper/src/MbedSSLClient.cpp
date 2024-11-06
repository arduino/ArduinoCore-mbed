#include "MbedSSLClient.h"

arduino::MbedSSLClient::MbedSSLClient()
  : _ca_cert_custom(nullptr),
    _hostname(nullptr),
    _disableSNI(false) {

  onBeforeConnect(mbed::callback(this, &MbedSSLClient::setRootCA));
};
