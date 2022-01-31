#include "GSM.h"

#include "mbed.h"
#include "CellularLog.h"
#include "CellularContext.h"
#include "CellularInterface.h"
#include "GEMALTO_CINTERION_CellularStack.h"

#define MAXRETRY 3

bool _cmuxEnable = false;
arduino::CMUXClass * arduino::CMUXClass::get_default_instance()
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

int arduino::GSMClass::begin(const char* pin, const char* apn, const char* username, const char* password, RadioAccessTechnologyType rat, bool restart) {

  if(restart || isCmuxEnable()) {
    pinMode(PJ_10, OUTPUT);
    digitalWrite(PJ_10, HIGH);
    delay(800);
    digitalWrite(PJ_10, LOW);
    pinMode(PJ_7, OUTPUT);
    digitalWrite(PJ_7, LOW);
    delay(1);
    digitalWrite(PJ_7, HIGH);
    delay(1);
    // this timer is to make sure that at boottime and when the CMUX is used,
    //  ^SYSTART is received in time to avoid stranger behaviour
    // from HW serial
    delay(2000);
  }

  _context = mbed::CellularContext::get_default_instance();

  if (_context == nullptr) {
    printf("Invalid context\n");
    return 0;
  }
  pinMode(PJ_7, INPUT_PULLDOWN);

  static mbed::DigitalOut rts(MBED_CONF_GEMALTO_CINTERION_RTS, 0);

  _device = _context->get_device();

  _device->set_cmux_status_flag(_cmuxGSMenable);

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

  int connect_status = NSAPI_ERROR_AUTH_FAILURE;
  uint8_t retryCount = 0;
  while(connect_status != NSAPI_ERROR_OK && retryCount < MAXRETRY) {

    connect_status = _context->connect(pin, apn, username, password);
    retryCount++;

    if (connect_status == NSAPI_ERROR_AUTH_FAILURE) {
      tr_info("Authentication Failure. Exiting application.\n");
    } else if (connect_status == NSAPI_ERROR_OK || connect_status == NSAPI_ERROR_IS_CONNECTED) {
      connect_status = NSAPI_ERROR_OK;
      tr_info("Connection Established.\n");
    } else if (retryCount > 2) {
      tr_info("Fatal connection failure: %d\n", connect_status);
    } else {
      tr_info("Couldn't connect, will retry...\n");
      continue;
    }

  }

  return connect_status == NSAPI_ERROR_OK ? 1 : 0;
}

void arduino::GSMClass::enableCmux(){
  _cmuxGSMenable = true;
}

bool arduino::GSMClass::isCmuxEnable(){
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

static Stream* trace_stream = nullptr;
static void arduino_print(const char* c) {
  if (trace_stream) {
    trace_stream->println(c);
  }
}

void arduino::GSMClass::debug(Stream& stream) {

#if MBED_CONF_MBED_TRACE_ENABLE

  mbed_trace_init();

  trace_stream = &stream;
  mbed_trace_print_function_set(arduino_print);
  mbed_trace_prefix_function_set( &trace_time );

  mbed_trace_mutex_wait_function_set(trace_wait);
  mbed_trace_mutex_release_function_set(trace_release);

  mbed_cellular_trace::mutex_wait_function_set(trace_wait);
  mbed_cellular_trace::mutex_release_function_set(trace_release);

#endif

}

NetworkInterface* arduino::GSMClass::getNetwork() {
  return _context;
}

arduino::GSMClass GSM;
