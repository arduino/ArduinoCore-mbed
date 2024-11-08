#include "MbedSSLClient.h"

arduino::MbedSSLClient::MbedSSLClient()
  : _ca_cert_custom(nullptr),
    _hostname(nullptr),
    _clientCert(nullptr),
    _privateKey(nullptr),
    _disableSNI(false),
    _appendCA(true) {

  onBeforeConnect(mbed::callback(this, &MbedSSLClient::setRootCA));
};
