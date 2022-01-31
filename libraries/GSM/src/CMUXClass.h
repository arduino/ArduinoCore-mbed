#ifndef _CMUXCLASS_H_
#define _CMUXCLASS_H_

extern "C"{
  #include "buffercmux.h"
}

#include "mbed.h"
#include "CellularLog.h"
#include "platform/FileHandle.h"
#include "drivers/SerialBase.h"
#include "drivers/UnbufferedSerial.h"
#include "platform/CircularBuffer.h"
#include "rtos/rtos.h"

namespace arduino {
static int nextSerialPort;
class CMUXClass {
public:
  CMUXClass(mbed::UnbufferedSerial* hw_serial);
  ~CMUXClass();
  void read();
  void write();
  int populate_tx_buffer(const char* buf, size_t sz, uint8_t id);
  void on_rx();
  static CMUXClass * get_default_instance();
  void enableCMUXChannel();

  mbed::FileHandle* get_serial(int index);
  void set_port(int id);
  int get_port();
  mbed::UnbufferedSerial * get_hw_serial();
  int cmux_handle_frame(char * temp_buf, int howMany);
  size_t writeInternal(void *buffer, size_t size);
  int write_frame(int channel, const char *input, int count, unsigned char type);
  int ussp_recv_data(const char *buf, int len, int port);
  int ussp_send_data(const char *buf, int n, int port);
  int findInBuf(char* buf, int len, char* needle);
  void handle_command(GSM0710_Frame * frame);
  int extract_frames(GSM0710_Buffer * buf);
  void setCmuxflag(bool cmuxFlag);
  void enableControlChannel();
  void enableGSMChannel();
  void enableGPSChannel();

  GSM0710_Buffer *in_buf = gsm0710_buffer_init();
  mbed::UnbufferedSerial * _hw_serial = nullptr;
  mbed::FileHandle * _gsm_serial = nullptr;
  mbed::FileHandle * _gps_serial = nullptr;

private:
  void api_lock();
  void api_unlock();
  PlatformMutex _mutex;
  bool _cmuxFlag = false;
  int id;
  mbed::CircularBuffer<char, 1500> rx_buffer;
  mbed::CircularBuffer<char, 1500> tx_buffer;
  rtos::Thread * reader_thd = new rtos::Thread(osPriorityNormal, 4096, nullptr, "CMUXt1");
};

}
#endif
