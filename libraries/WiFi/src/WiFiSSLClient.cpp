#include "WiFiSSLClient.h"

arduino::WiFiSSLClient::WiFiSSLClient() {
    onBeforeConnect(mbed::callback(this, &WiFiSSLClient::setRootCA));
};