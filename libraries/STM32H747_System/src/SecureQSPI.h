#include "Arduino.h"
#include "QSPIFBlockDevice.h"

class SecureQSPIFBlockDevice: public QSPIFBlockDevice {
  public:
    virtual int readSecure(void *buffer, mbed::bd_addr_t addr, mbed::bd_size_t size) {
      int ret = 0;
      ret &= _qspi.command_transfer(0xB1, -1, nullptr, 0, nullptr, 0);
      ret &= read(buffer, addr, size);
      ret &= _qspi.command_transfer(0xC1, -1, nullptr, 0, nullptr, 0);
      return ret;
    }
};