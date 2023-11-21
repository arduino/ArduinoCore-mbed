/*
  GSM.cpp - Library for GSM on mbed platforms.
  Copyright (c) 2011-2023 Arduino LLC.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "GSM.h"

#include "mbed.h"
#include "CellularLog.h"
#include "CellularDevice.h"
#include "CellularContext.h"
#include "CellularInterface.h"
#include "GEMALTO_CINTERION_CellularStack.h"

arduino::CMUXClass *arduino::CMUXClass::get_default_instance()
{
  static mbed::UnbufferedSerial serial(MBED_CONF_GEMALTO_CINTERION_TX, MBED_CONF_GEMALTO_CINTERION_RX, 115200);
  serial.set_flow_control(mbed::SerialBase::RTSCTS_SW, MBED_CONF_GEMALTO_CINTERION_CTS, NC);
  static arduino::CMUXClass device(&serial);
  return &device;
}

mbed::CellularDevice *mbed::CellularDevice::get_default_instance()
{
  static auto cmux = arduino::CMUXClass::get_default_instance();
  static mbed::GEMALTO_CINTERION device(cmux->get_serial(0));
  nextSerialPort++;
  device.enableCMUXChannel = mbed::callback(cmux, &arduino::CMUXClass::enableCMUXChannel);
  return &device;
}

int arduino::GSMClass::begin(const char* pin, const char* apn, const char* username, const char* password, RadioAccessTechnologyType rat, uint32_t band, bool restart) {

  if (restart || isCmuxEnable()) {
    reset();
  }

  _context = mbed::CellularContext::get_default_instance();

  if (_context == nullptr) {
    DEBUG_ERROR("Invalid mbed::CellularContext");
    return 0;
  }

  pinMode(MBED_CONF_GEMALTO_CINTERION_ON, INPUT_PULLDOWN);

  static mbed::DigitalOut rts(MBED_CONF_GEMALTO_CINTERION_RTS, 0);

  _device = _context->get_device();
  _device->modem_debug_on(_at_debug);

  if (!isReady()) {
    DEBUG_ERROR("Cellular device not ready");
    return 0;
  }

  _device->set_cmux_status_flag(_cmuxGSMenable);
  _device->set_retry_timeout_array(_retry_timeout, sizeof(_retry_timeout) / sizeof(_retry_timeout[0]));
#if GSM_DEBUG_ENABLE
  _device->attach(mbed::callback(this, &GSMClass::onStatusChange));
#endif
  _device->init();

  _pin = pin;
  _apn = apn;
  _username = username;
  _password = password;
  _rat = rat;
  _band = (FrequencyBand) band;

  _context->set_sim_pin(pin);
  _context->set_authentication_type(mbed::CellularContext::AuthenticationType::PAP);
  _context->set_credentials(_apn, _username, _password);
  _context->set_access_technology(_rat);
  _context->set_band(_band);

  int connect_status = NSAPI_ERROR_AUTH_FAILURE;

  DEBUG_INFO("Connecting...");
  connect_status = _context->connect(pin, apn, username, password);

  if (connect_status == NSAPI_ERROR_AUTH_FAILURE) {
    DEBUG_ERROR("Authentication Failure. Exiting application.");
  } else if (connect_status == NSAPI_ERROR_OK || connect_status == NSAPI_ERROR_IS_CONNECTED) {
    connect_status = NSAPI_ERROR_OK;
    DEBUG_INFO("Connection Established.");
  } else {
    DEBUG_ERROR("Couldn't connect.");
  }

  return connect_status == NSAPI_ERROR_OK ? 1 : 0;
}

void arduino::GSMClass::enableCmux() {
  _cmuxGSMenable = true;
}

bool arduino::GSMClass::isCmuxEnable() {
  return _cmuxGSMenable;
}

void arduino::GSMClass::end() {

}

int arduino::GSMClass::disconnect() {
  return _context->disconnect();
}

unsigned long arduino::GSMClass::getTime()
{
  return _device->get_time();
}

unsigned long arduino::GSMClass::getLocalTime()
{
  return _device->get_local_time();
}

bool arduino::GSMClass::setTime(unsigned long const epoch, int const timezone)
{
  return _device->set_time(epoch, timezone);
}

bool arduino::GSMClass::isConnected()
{
  if (_context) {
    return _context->is_connected();
  } else {
    return false;
  }
}



NetworkInterface* arduino::GSMClass::getNetwork() {
  return _context;
}

void arduino::GSMClass::reset() {
  pinMode(MBED_CONF_GEMALTO_CINTERION_RST, OUTPUT);
  digitalWrite(MBED_CONF_GEMALTO_CINTERION_RST, HIGH);
  delay(800);
  digitalWrite(MBED_CONF_GEMALTO_CINTERION_RST, LOW);
  pinMode(MBED_CONF_GEMALTO_CINTERION_ON, OUTPUT);
  digitalWrite(MBED_CONF_GEMALTO_CINTERION_ON, LOW);
  delay(1);
  digitalWrite(MBED_CONF_GEMALTO_CINTERION_ON, HIGH);
  delay(1);
}

bool arduino::GSMClass::isReady(const int timeout) {
  if (!_device) {
    DEBUG_ERROR("No device found");
    return false;
  }

  const unsigned int start = millis();
  while (_device->is_ready() != NSAPI_ERROR_OK) {

    if (millis() - start > timeout) {
      DEBUG_WARNING("Timeout waiting device ready");
      return false;
    }
    delay(100);
  }
  return true;
}

arduino::GSMClass GSM;
