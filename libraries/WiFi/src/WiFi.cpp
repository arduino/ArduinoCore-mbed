#include "WiFi.h"

#define SSID_MAX_LENGTH 32

bool arduino::WiFiClass::isVisible(const char* ssid) {
  for (int i = 0; i < 10; i++) {
    if (strncmp(ap_list[i].get_ssid(), ssid, SSID_MAX_LENGTH) == 0) {
      connected_ap = i;
      return true;
    }
  }
  return false;
}

int arduino::WiFiClass::begin(const char* ssid, const char* passphrase) {
  if (wifi_if == nullptr) {
    return 0;
  }

  scanNetworks();
  // use scan result to populate security field
  if (!isVisible(ssid)) {
    _currentNetworkStatus = WL_CONNECT_FAILED;
    return _currentNetworkStatus;
  }

  nsapi_error_t result = wifi_if->connect(ssid, passphrase, ap_list[connected_ap].get_security());

  _currentNetworkStatus = (result == NSAPI_ERROR_OK && setSSID(ssid)) ? WL_CONNECTED : WL_CONNECT_FAILED;
  return _currentNetworkStatus;
}

//Config Wifi to set Static IP && Disable DHCP
void arduino::WiFiClass::config(const char* localip, const char* netmask, const char* gateway){
  wifi_if->set_network(localip, netmask, gateway);
  wifi_if->set_dhcp(false);
}

int arduino::WiFiClass::beginAP(const char* ssid, const char* passphrase, uint8_t channel) {

#if defined(COMPONENT_4343W_FS)
  _softAP = WhdSoftAPInterface::get_default_instance();
#endif

  if (_softAP == NULL) {
    _currentNetworkStatus = WL_AP_FAILED;
    return _currentNetworkStatus;
  }

  ensureDefaultAPNetworkConfiguration();

  WhdSoftAPInterface* softAPInterface = static_cast<WhdSoftAPInterface*>(_softAP);

  //Set ap ssid, password and channel
  softAPInterface->set_network(_ip, _netmask, _gateway);
  nsapi_error_t result = softAPInterface->start(ssid, passphrase, NSAPI_SECURITY_WPA2, channel, true /* dhcp server */, NULL, true /* cohexistance */);

  nsapi_error_t registrationResult;
  softAPInterface->unregister_event_handler();
  registrationResult = softAPInterface->register_event_handler(arduino::WiFiClass::handleAPEvents);

  if (registrationResult != NSAPI_ERROR_OK) {
    _currentNetworkStatus = WL_AP_FAILED;
    return _currentNetworkStatus;
  }

  _currentNetworkStatus = (result == NSAPI_ERROR_OK && setSSID(ssid)) ? WL_AP_LISTENING : WL_AP_FAILED;
  return _currentNetworkStatus;
}

void* arduino::WiFiClass::handleAPEvents(whd_interface_t ifp, const whd_event_header_t* event_header, const uint8_t* event_data, void* handler_user_data) {
  if (event_header->event_type == WLC_E_ASSOC_IND) {
    WiFi._currentNetworkStatus = WL_AP_CONNECTED;
  } else if (event_header->event_type == WLC_E_DISASSOC_IND) {
    WiFi._currentNetworkStatus = WL_AP_LISTENING;
  }

  // Default Event Handler
  whd_driver_t whd_driver = ifp->whd_driver;
  WHD_IOCTL_LOG_ADD_EVENT(whd_driver, event_header->event_type, event_header->flags, event_header->reason);

  if ((event_header->event_type == (whd_event_num_t)WLC_E_LINK) || (event_header->event_type == WLC_E_IF)) {
    if (osSemaphoreGetCount(whd_driver->ap_info.whd_wifi_sleep_flag) < 1) {
      osStatus_t result = osSemaphoreRelease(whd_driver->ap_info.whd_wifi_sleep_flag);
      if (result != osOK) {
        printf("Release whd_wifi_sleep_flag ERROR: %d", result);
      }
    }
  }

  return handler_user_data;
}

void arduino::WiFiClass::ensureDefaultAPNetworkConfiguration() {
  if (_ip == nullptr) {
    _ip = SocketAddress(DEFAULT_IP_ADDRESS);
  }
  if (_gateway == nullptr) {
    _gateway = _ip;
  }
  if (_netmask == nullptr) {
    _netmask = SocketAddress(DEFAULT_NETMASK);
  }
}

void arduino::WiFiClass::end() {
  disconnect();
  _softAP = nullptr;
}

int arduino::WiFiClass::disconnect() {
  if (_softAP != nullptr) {
    WhdSoftAPInterface* softAPInterface = static_cast<WhdSoftAPInterface*>(_softAP);
    softAPInterface->unregister_event_handler();
    _currentNetworkStatus = (softAPInterface->stop() == NSAPI_ERROR_OK ? WL_DISCONNECTED : WL_AP_FAILED);
  } else {
    wifi_if->disconnect();
    _currentNetworkStatus = WL_DISCONNECTED;
  }

  return _currentNetworkStatus;
}

char* arduino::WiFiClass::SSID() {
  return _ssid;
}

int arduino::WiFiClass::setSSID(const char* ssid) {
  if (_ssid) free(_ssid);

  _ssid = (char*)malloc(SSID_MAX_LENGTH + 1);
  if (!_ssid) {
    //tr_error("Could not allocate ssid buffer");
    return 0;
  }

  memcpy(_ssid, ssid, SSID_MAX_LENGTH + 1);
  // too long? break it off
  if (strlen(ssid) > SSID_MAX_LENGTH) _ssid[SSID_MAX_LENGTH] = 0;
  return 1;
}

static uint8_t sec2enum(nsapi_security_t sec) {
  switch (sec) {
    case NSAPI_SECURITY_NONE:
      return ENC_TYPE_NONE;
    case NSAPI_SECURITY_WEP:
      return ENC_TYPE_WEP;
    case NSAPI_SECURITY_WPA:
      return ENC_TYPE_TKIP;
    case NSAPI_SECURITY_WPA2:
      return ENC_TYPE_CCMP;
    case NSAPI_SECURITY_WPA_WPA2:
      return ENC_TYPE_CCMP;
    case NSAPI_SECURITY_UNKNOWN:
    default:
      return ENC_TYPE_AUTO;
  }
}

int8_t arduino::WiFiClass::scanNetworks() {
  uint8_t count = 10;
  if (ap_list == nullptr) {
    ap_list = new WiFiAccessPoint[count];
  }
  return wifi_if->scan(ap_list, count);
}

char* arduino::WiFiClass::SSID(uint8_t networkItem) {
  return (char*)ap_list[networkItem].get_ssid();
}

int32_t arduino::WiFiClass::RSSI(uint8_t networkItem) {
  return ap_list[networkItem].get_rssi();
}

uint8_t arduino::WiFiClass::encryptionType(uint8_t networkItem) {
  return sec2enum(ap_list[networkItem].get_security());
}

int32_t arduino::WiFiClass::RSSI() {
  return wifi_if->get_rssi();
}

uint8_t arduino::WiFiClass::status() {
  return _currentNetworkStatus;
}

uint8_t arduino::WiFiClass::encryptionType() {
  return sec2enum(ap_list[connected_ap].get_security());
}

uint8_t* arduino::WiFiClass::BSSID(unsigned char* bssid) {
  const uint8_t* reverse_bssid = ap_list[connected_ap].get_bssid();
  for (int b = 0; b < 6; b++) {
    bssid[b] = reverse_bssid[5 - b];
  }
  return bssid;
}

NetworkInterface* arduino::WiFiClass::getNetwork() {
  if (_softAP != nullptr) {
    return _softAP;
  } else {
    return wifi_if;
  }
}

unsigned long arduino::WiFiClass::getTime() {
  return 0;
}

#if defined(COMPONENT_4343W_FS)

#define WIFI_FIRMWARE_PATH "/wlan/4343WA1.BIN"

bool firmware_available = false;

#include "wiced_filesystem.h"
#include "resources.h"

void wiced_filesystem_mount_error(void) {
  Serial.println("Failed to mount the filesystem containing the WiFi firmware.");
  Serial.println("Usually that means that the WiFi firmware has not been installed yet"
                 " or was overwritten with another firmware.");
  whd_print_logbuffer();
  while (1) {}
}

void wiced_filesystem_firmware_error(void) {
  Serial.println("Please run the \"WiFiFirmwareUpdater\" sketch once to install the WiFi firmware.");
  whd_print_logbuffer();
  while (1) {}
}

wiced_result_t whd_firmware_check_hook(const char* mounted_name, int mount_err) {
  DIR* dir;
  struct dirent* ent;
  String dir_name(mounted_name);
  if (mount_err) {
    wiced_filesystem_mount_error();
  } else {
    if ((dir = opendir(mounted_name)) != NULL) {
      // print all the files and directories within directory
      while ((ent = readdir(dir)) != NULL) {
        String fullname = "/" + dir_name + "/" + String(ent->d_name);
        if (fullname == WIFI_FIRMWARE_PATH) {
          closedir(dir);
          firmware_available = true;
          return WICED_SUCCESS;
        }
      }
      Serial.println("File not found");
      closedir(dir);
    }
    wiced_filesystem_firmware_error();
  }
  return WICED_ERROR;
}


#include "whd_version.h"
const char* arduino::WiFiClass::firmwareVersion() {
  if (firmware_available) {
    return WHD_VERSION;
  } else {
    return "v0.0.0";
  }
}

arduino::WiFiClass WiFi(WiFiInterface::get_default_instance());
#endif

// every specialization library should declare its own WiFI object: eg
//
// static ESP8266Interface wifi_if(PD_8, PD_9);
// arduino::WiFiClass WiFi(&wifi_if);
