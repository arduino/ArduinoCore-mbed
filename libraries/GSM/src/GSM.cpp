#include "GSM.h"

#include "mbed.h"
#include "CellularLog.h"

#define SSID_MAX_LENGTH 32

#define MAXRETRY 3

mbed::CellularDevice *mbed::CellularDevice::get_default_instance()
{
  static BufferedSerial serial(MBED_CONF_GEMALTO_CINTERION_TX, MBED_CONF_GEMALTO_CINTERION_RX, 115200);
#if defined(MBED_CONF_GEMALTO_CINTERION_RTS) && defined(MBED_CONF_GEMALTO_CINTERION_CTS)
  serial.set_flow_control(mbed::SerialBase::RTSCTS, MBED_CONF_GEMALTO_CINTERION_RTS, MBED_CONF_GEMALTO_CINTERION_CTS);
#endif
  static GEMALTO_CINTERION device(&serial);
  return &device;
}

int arduino::GSMClass::begin(const char* pin, const char* apn, const char* username, const char* password, RadioAccessTechnologyType rat) {
  if (gsm_if == nullptr) {
    printf("Invalid gsm_if\n");
    return 0;
  }

  _context->set_sim_pin(pin);

  _device->init();

  _context->set_authentication_type((mbed::CellularContext::AuthenticationType)1);

  _pin = pin;
  _apn = apn;
  _username = username;
  _password = password;
  _rat = rat;
  _context->set_credentials(apn, username, password);

  _context->set_access_technology(rat);

  uint8_t connect_status = NSAPI_ERROR_AUTH_FAILURE;
  uint8_t retryCount = 0;
  while(connect_status != NSAPI_ERROR_OK && retryCount < MAXRETRY) {

    connect_status = _context->connect(pin, apn, username, password);
    retryCount++;

    if (connect_status == NSAPI_ERROR_AUTH_FAILURE) {
      printf("Authentication Failure. Exiting application.\n");
    } else if (connect_status == NSAPI_ERROR_OK || connect_status == NSAPI_ERROR_IS_CONNECTED) {
      connect_status = NSAPI_ERROR_OK;
      printf("Connection Established.\n");
    } else if (retryCount > 2) {
      printf("Fatal connection failure: %d\n", connect_status);
    } else {
      printf("Couldn't connect, will retry...\n");
      continue;
    }

  }

  return connect_status;
}

void arduino::GSMClass::end() {

}

int arduino::GSMClass::disconnect() {

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

static PlatformMutex trace_mutex;

static void trace_wait()
{
    trace_mutex.lock();
}

static void trace_release()
{
    trace_mutex.unlock();
}

static char* trace_time(size_t ss)
{
    static char time_st[50];
    auto ms = std::chrono::time_point_cast<std::chrono::milliseconds>(rtos::Kernel::Clock::now()).time_since_epoch().count();
    //snprintf(time_st, 49, "[%08llums]", ms);
    snprintf(time_st, 1, "\n");
    return time_st;
}


void arduino::GSMClass::debug() {

#ifndef CELLULAR_DEMO_TRACING_H_
#define CELLULAR_DEMO_TRACING_H_

#if MBED_CONF_MBED_TRACE_ENABLE

    mbed_trace_init();
    mbed_trace_prefix_function_set( &trace_time );

    mbed_trace_mutex_wait_function_set(trace_wait);
    mbed_trace_mutex_release_function_set(trace_release);

    mbed_cellular_trace::mutex_wait_function_set(trace_wait);
    mbed_cellular_trace::mutex_release_function_set(trace_release);
#endif
#endif

}

NetworkInterface* arduino::GSMClass::getNetwork() {
  return gsm_if;
}

arduino::GSMClass GSM(mbed::CellularContext::get_default_instance());
