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
#include "GSMDebug.h"

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

  /* Assume module is powered ON. Uncomment this line is you are using
   * Edge Control without Arduino_ConnectionHandler
   * #if defined (ARDUINO_EDGE_CONTROL)
   *   pinMode(ON_MKR2, OUTPUT);
   *   digitalWrite(ON_MKR2, HIGH);
   * #endif
   */

  /* Ensure module is not under reset */
  pinMode(MBED_CONF_GEMALTO_CINTERION_RST, OUTPUT);
  digitalWrite(MBED_CONF_GEMALTO_CINTERION_RST, LOW);

  /* Reset module if needed */
  const bool emergencyReset = restart || isCmuxEnable();
  DEBUG_INFO("Emergency reset %s", emergencyReset ? "enabled" : "disabled");
  if (emergencyReset) {
    hardwareReset();
  }

  /* Create rising edge on pin ON */
  on();

  if (!_context) {
    _context = mbed::CellularContext::get_default_instance();
  }

  if (_context == nullptr) {
    DEBUG_ERROR("Invalid mbed::CellularContext");
    return 0;
  }

#if defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_PORTENTA_H7_M4)
  /* This is needed to wakeup module if hw flow control is enabled */
  static mbed::DigitalOut rts(MBED_CONF_GEMALTO_CINTERION_RTS, 0);
#endif

  _device = _context->get_device();
  _device->modem_debug_on(_at_debug);

  DEBUG_INFO("CMUX %s", _cmuxGSMenable ? "enabled" : "disabled");
  _device->set_cmux_status_flag(_cmuxGSMenable);
  _device->set_retry_timeout_array(_retry_timeout, sizeof(_retry_timeout) / sizeof(_retry_timeout[0]));
  _device->set_timeout(_timeout);
  _device->attach(mbed::callback(this, &GSMClass::onStatusChange));
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

void arduino::GSMClass::setTimeout(unsigned long timeout) {
  _timeout = timeout;
}

void arduino::GSMClass::enableCmux() {
  _cmuxGSMenable = true;
}

bool arduino::GSMClass::isCmuxEnable() {
  return _cmuxGSMenable;
}

void arduino::GSMClass::end() {
  if(_device) {
    _device->shutdown();
  }
}

void arduino::GSMClass::reset() {
  if(_device) {
    _device->soft_reset();
  }
}

void arduino::GSMClass::off() {
  if(_device) {
    _device->soft_power_off();
  }
}

int arduino::GSMClass::ping(const char* hostname, int ttl) {

  mbed::GEMALTO_CINTERION_CellularStack* stack = (mbed::GEMALTO_CINTERION_CellularStack*)_context->get_stack();
  if (!stack) {
    return -1;
  }
  return stack->ping(hostname, ttl);
}

int arduino::GSMClass::ping(const String &hostname, int ttl)
{
  return ping(hostname.c_str(), ttl);
}

int arduino::GSMClass::ping(IPAddress ip, int ttl)
{
  String host;
  host.reserve(15);

  host += ip[0];
  host += '.';
  host += ip[1];
  host += '.';
  host += ip[2];
  host += '.';
  host += ip[3];

  return ping(host, ttl);
}

int arduino::GSMClass::disconnect() {
  if (!_context) {
    return 0;
  }

  if (_context->is_connected()) {
    return _context->disconnect();
  }

  return 0;
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
  /* Can happen this is called before GSM.begin( .. ) when configuring GSMSSLClient
   * from sketch calling client.appendCustomCACert( .. ) */
  if (!_context) {
    _context = mbed::CellularContext::get_default_instance();
  }
  return _context;
}

void arduino::GSMClass::hardwareReset() {
  /* Reset logic is inverted */
  pinMode(MBED_CONF_GEMALTO_CINTERION_RST, OUTPUT);
  digitalWrite(MBED_CONF_GEMALTO_CINTERION_RST, HIGH);
  delay(800);
  digitalWrite(MBED_CONF_GEMALTO_CINTERION_RST, LOW);
}

void arduino::GSMClass::on() {
  /* Module needs a rising edge to power on */
  pinMode(MBED_CONF_GEMALTO_CINTERION_ON, OUTPUT);
  digitalWrite(MBED_CONF_GEMALTO_CINTERION_ON, LOW);
  delay(1);
  digitalWrite(MBED_CONF_GEMALTO_CINTERION_ON, HIGH);
  delay(1);
}


arduino::GSMClass GSM;
