#include "GPS.h"

#include "mbed.h"

arduino::GPSClass::GPSClass()
{
}

arduino::GPSClass::~GPSClass()
{
}

int arduino::GPSClass::available(void)
{
  return _serial->available();
}

void arduino::GPSClass::readAndPrint()
{
  char buf[256];
  int dataRead = 0;
  while((dataRead = available()) <= 0);

  String tmp = "";
  while((dataRead =_serial->read(buf, 1)) > 0){
    tmp += String(buf);
  }
  Serial.println(tmp);
}

bool arduino::GPSClass::checkGNSSEngine(const char *prefix)
{
  char buf[256];
  int dataRead = 0;
  while((dataRead = available()) <= 0);

  String tmp = "";
  while((dataRead =_serial->read(buf, 1)) > 0){
    tmp += String(buf);
  }

  int offset = 0;
  // check the buffer in serch of prefix
  while (sizeof(tmp) - offset > sizeof(prefix))
  {
    if (tmp.substring(offset).startsWith(String(prefix)))
    {
      // read all the remaining data if available
      _serial->read(buf, sizeof(buf));
      return true;
    }
    offset++;
  }
  return false;
}

void arduino::GPSClass::readAndDrop()
{
  char buf[256];
  uint32_t start = millis();
  int dataRead = 0;
  while (!available() && millis() - start < 1000) {}

  String tmp = "";
  while(_serial->read(buf, 1) > 0);
}

void arduino::GPSClass::begin(unsigned long baudrate) {
  begin(baudrate, 0);
}


void arduino::GPSClass::begin(unsigned long baudrate, uint16_t config)
{
  auto cmux = arduino::CMUXClass::get_default_instance();

  auto serial = cmux->get_serial(1);
  _serial = (arduino::PTYSerial *)serial;
  nextSerialPort++;

  _serial->write("ATE0\r\n", sizeof("ATE0\r\n"));
  readAndDrop();

  do
  {
    _serial->write("AT^SPIO=0\r\n", sizeof("AT^SPIO=0\r\n"));
    readAndDrop();

    _serial->write("AT^SPIO=1\r\n", sizeof("AT^SPIO=1\r\n"));
    readAndDrop();

    _serial->write("AT^SCPIN=1,7,1,0\r\n", sizeof("AT^SCPIN=1,7,1,0\r\n"));
    readAndDrop();

    _serial->write("AT^SSIO=7,1\r\n", sizeof("AT^SSIO=7,1\r\n"));
    readAndDrop();

    _serial->write("AT^SGIO=7\r\n", sizeof("AT^SGIO=7\r\n"));
  } while (!checkGNSSEngine("^SGIO: 1"));

  _serial->write("AT^SGPSC=Engine/StartMode,0\r\n", sizeof("AT^SGPSC=Engine/StartMode,0\r\n"));
  checkGNSSEngine("^SGPSC: \"Engine/StartMode\",\"0\"");

  while (!_engine)
  {
    _serial->write("AT^SGPSC=Engine,3\r\n", sizeof("AT^SGPSC=Engine,3\r\n"));
    _engine = checkGNSSEngine("^SGPSC: \"Engine\",\"3\"");
  }

  _serial->write("AT^SGPSC=Nmea/Urc,on\r\n", sizeof("AT^SGPSC=Nmea/Urc,on\r\n"));
  readAndDrop();
}

int arduino::GPSClass::peek(void)
{
  return read();
}

int arduino::GPSClass::read(void)
{
  char c;
  _serial->read(&c,1);
  return c;
}

void arduino::GPSClass::flush(void)
{
}

size_t arduino::GPSClass::write(char *buffer)
{
  size_t sz = sizeof(buffer);
  return _serial->write(buffer, sz);
}

size_t arduino::GPSClass::write(char *buffer, size_t sz)
{
  size_t data_write = 0;
  _serial->write(buffer, sz);
  return data_write;
}

size_t arduino::GPSClass::write(uint8_t c)
{
  return _serial->write((char *)&c, 1);
}

void arduino::GPSClass::end()
{
  _serial->write("AT^SGPSC=Nmea/Urc,off\r\n", sizeof("AT^SGPSC=Nmea/Urc,off\r\n"));
  readAndDrop();

  while (_engine)
  {
    _serial->write("AT^SGPSC=Engine,0\r\n", sizeof("AT^SGPSC=Engine,0\r\n"));
    _engine = !checkGNSSEngine("^SGPSC: \"Engine\",\"0\"");
  }

  _serial->write("AT^SSIO=7,0\r\n", sizeof("AT^SSIO=7,0\r\n"));
  readAndDrop();
}

arduino::GPSClass::operator bool()
{
  _serial->write("AT^SGPSC?\r\n", sizeof("AT^SGPSC?\r\n"));
  return checkGNSSEngine("\"Engine\",\"3\"");
}

extern Stream *trace_stream;
static void arduino_print(const char *c)
{
  if (trace_stream)
  {
    trace_stream->println(c);
  }
}

arduino::GPSClass GPS;
