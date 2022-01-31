#include "CMUXClass.h"
#include "PTYSerial.h"
namespace arduino {

typedef struct Channel_Status {
  int opened;
  unsigned char v24_signals;
} Channel_Status;

#define DEFAULT_NUMBER_OF_PORTS 3
#define WRITE_RETRIES 5
#define MAX_CHANNELS 32

#define ICANON 0000002
#define ECHO 0000010
#define ECHOE 0000020
#define ISIG 0000001
#define INLCR 0000100
#define ICRNL 0000400
#define IGNCR 0000200
#define OPOST 0000001
#define OLCUC 0000002
#define ONLRET 0000040
#define ONOCR 0000020
#define OCRNL 0000010
#define TCSANOW 0
#define CLOCAL 0004000
#define CREAD 0000200
#define CS8 0000060
#define HUPCL 0002000
#define IGNBRK 0000001
#define PARENB 0000400
#define CSTOPB 0000100
#define CSIZE 0000060
#define B9600 0000015
#define B19200 0000016
#define B38400 0000017
#define B57600 0010001
#define B115200 0010002
#define B230400 0010003
#define B460800 0010004
#define NCCS 32

#ifndef min
#define min(a, b) ((a < b) ? a : b)
#endif

static volatile int terminate = 0;
static int terminateCount = 0;
static Channel_Status *cstatus;
static int max_frame_size = 31; // The limit of Sony-Ericsson GM47

static GSM0710_Buffer *in_buf = gsm0710_buffer_init(); // input buffer


static int faultTolerant = 0;
static int restart = 0;

#define COMMAND_IS(command, type) ((type & ~CR_CMD) == command)
#define PF_ISSET(frame) ((frame->control & PF) == PF)
#define FRAME_IS(type, frame) ((frame->control & ~PF) == type)

CMUXClass::CMUXClass(mbed::UnbufferedSerial* hw_serial) : _hw_serial(hw_serial) {
  _gsm_serial = new arduino::PTYSerial(this);
  _gps_serial = new arduino::PTYSerial(this);
  nextSerialPort = 0;
  cstatus = (Channel_Status *) malloc(sizeof(Channel_Status) * (1 + 3));
  reader_thd->start(mbed::callback(this, &CMUXClass::read));
  _hw_serial->attach(mbed::callback(this, &CMUXClass::on_rx), mbed::SerialBase::RxIrq);
}

CMUXClass::~CMUXClass(){

}

void CMUXClass::enableCMUXChannel()
{
  enableControlChannel();
  enableGSMChannel();
  enableGPSChannel();
  setCmuxflag(true);
}

void CMUXClass::enableControlChannel(){
  cstatus[0].opened = 0;
  cstatus[0].v24_signals = S_DV | S_RTR | S_RTC | EA;
  api_lock();
  write_frame(0, NULL, 0, SABM | PF);
  api_unlock();
}

void CMUXClass::enableGSMChannel(){
  cstatus[1].opened = 0;
  cstatus[1].v24_signals = S_DV | S_RTR | S_RTC | EA;
  api_lock();
  write_frame(1, NULL, 0, SABM | PF);
  api_unlock();
}

void CMUXClass::enableGPSChannel(){
  api_lock();
  cstatus[2].opened = 0;
  cstatus[2].v24_signals = S_DV | S_RTR | S_RTC | EA;
  write_frame(3, NULL, 0, SABM | PF);
  api_unlock();
}

void CMUXClass::setCmuxflag(bool cmuxFlag){
  _cmuxFlag = cmuxFlag;
}

void CMUXClass::on_rx() {
  while(_hw_serial->readable()) {
    char c;
    _hw_serial->read(&c, 1);
    rx_buffer.push(c);
  }
  osSignalSet(reader_thd->get_id(), 0x01);
}


int CMUXClass::cmux_handle_frame(char * temp_buf, int howMany) {
  gsm0710_buffer_write(in_buf, temp_buf, howMany);
  extract_frames(in_buf);
  return howMany;
}

void CMUXClass::read() {
  while (1) {
    osSignalWait(0, osWaitForever);
    char temp_buf[1500];
    size_t howMany = rx_buffer.size();
    this->writeInternal(temp_buf, howMany);
    if (howMany > 0){
      if(_cmuxFlag){
        cmux_handle_frame(temp_buf, howMany);
      } else {
        ((PTYSerial *)_gsm_serial)->populate_rx_buffer(temp_buf, howMany);
      }
    }
  }
}

size_t CMUXClass::writeInternal(void *buffer, size_t size)
{
  char *buf = static_cast<char *>(buffer);

  if (size == 0) {
    return 0;
  }

  bool lock_api = !core_util_in_critical_section();

  if (lock_api) {
    api_lock();
  }
  for (size_t i = 0; i < size; i++) {
    rx_buffer.pop(buf++,1);
  }

  if (lock_api) {
    api_unlock();
  }

  return size;
}

int CMUXClass::populate_tx_buffer(const char* buf, size_t sz, uint8_t id) {
  int ret = 0;
  api_lock();
  if(_cmuxFlag){
    ret = write_frame(id + 1 , buf ,sz, UIH);
  } else {
    ret = _hw_serial->write(buf, sz);
  }
  api_unlock();
  return ret;
}

void CMUXClass::api_lock(){
  _mutex.lock();
}

void CMUXClass::api_unlock(){
  _mutex.unlock();
}

mbed::UnbufferedSerial * CMUXClass::get_hw_serial() {
  return _hw_serial;
}

mbed::FileHandle* CMUXClass::get_serial(int index) {
  if (index == 0) {
    ((PTYSerial *)_gsm_serial)->set_port(0);
    return _gsm_serial;
  }
  if (index == 1) {
    ((PTYSerial *)_gps_serial)->set_port(2);
    return _gps_serial;
  }
  return nullptr;
}

int CMUXClass::write_frame(int channel, const char *input, int count, unsigned char type)
{
  // flag, EA=1 C channel, frame type, length 1-2
  unsigned char prefix[5] = {F_FLAG, EA | CR_CMD, 0, 0, 0};
  unsigned char postfix[2] = {0xFF, F_FLAG};
  int prefix_length = 4, c;

  tr_debug("send frame to ch: %d \n", channel);

  // EA=1, Command, let's add address
  prefix[1] = prefix[1] | ((63 & (unsigned char)channel) << 2);
  // let's set control field
  prefix[2] = type;

  // let's not use too big frames
  count = min(max_frame_size, count);

  // length
  if (count > 127)
  {
    prefix_length = 5;
    prefix[3] = ((127 & count) << 1);
    prefix[4] = (32640 & count) >> 7;
  }
  else
  {
    prefix[3] = 1 | (count << 1);
  }
  // CRC checksum
  postfix[0] = make_fcs(prefix + 1, prefix_length - 1);

  c = _hw_serial->write(prefix, prefix_length);
  if (c != prefix_length)
  {
    tr_debug("Couldn't write the whole prefix to the serial port for the virtual port %d. Wrote only %d  bytes.", channel, c);
    return 0;
  }
  if (count > 0)
  {
    c = _hw_serial->write(input, count);
    if (count != c)
    {
      tr_debug("Couldn't write all data to the serial port from the virtual port %d. Wrote only %d bytes.\n", channel, c);
      return 0;
    }
  }
  c = _hw_serial->write(postfix, 2);
  if (c != 2)
  {
    tr_debug("Couldn't write the whole postfix to the serial port for the virtual port %d. Wrote only %d bytes.", channel, c);
    return 0;
  }

  return count;
}

/* Handles received data from ussp device.
*
* This function is derived from a similar function in RFCOMM Implementation
* with USSPs made by Marcel Holtmann.
*
* PARAMS:
* buf   - buffer, which contains received data
* len   - the length of the buffer
* port  - the number of ussp device (logical channel), where data was
*         received
* RETURNS:
* the number of remaining bytes in partial packet
*/

int CMUXClass::ussp_recv_data(const char *buf, int len, int port)
{
  int written = 0;
  int i = 0;
  int last = 0;
  api_lock();
  // try to write 5 times
  while (written != len && i < WRITE_RETRIES)
  {
    last = write_frame(port + 1, buf + written, len - written, UIH);
    written += last;
    if (last == 0)
    {
      i++;
    }
  }
  api_unlock();
  if (i == WRITE_RETRIES)
  {
    tr_debug("Couldn't write data to channel %d. Wrote only %d bytes, when should have written %ld.\n", (port + 1), written, (long)len);
  }
  return 0;
}

int CMUXClass::ussp_send_data(const char *buf, int n, int port)
{
  if (port == 1)
  {
    tr_debug("send data to port virtual port %d\n", port);
    int ret = ((PTYSerial *)_gsm_serial)->populate_rx_buffer(buf, n);
    return ret;
  }

  if (port == 2 || port == 3)
  {
    tr_debug("send data to port virtual port %d\n", port);
    int ret = ((PTYSerial *)_gps_serial)->populate_rx_buffer(buf, n);
    return ret;
  }
  return 0;
}

// Returns 1 if found, 0 otherwise. needle must be null-terminated.
// strstr might not work because WebBox sends garbage before the first OK
int CMUXClass::findInBuf(char *buf, int len, char *needle)
{
  int i;
  int needleMatchedPos = 0;

  if (needle[0] == '\0')
  {
  return 1;
  }

  for (i = 0; i < len; i++)
  {
    if (needle[needleMatchedPos] == buf[i])
    {
    needleMatchedPos++;
      if (needle[needleMatchedPos] == '\0')
      {
        // Entire needle was found
        return 1;
      }
    }
    else
    {
      needleMatchedPos = 0;
    }
  }
  return 0;
}

/* Handles commands received from the control channel.
*/
void CMUXClass::handle_command(GSM0710_Frame *frame) // guardare la gestionme dei comandi
{
  unsigned char type, signals;
  int length = 0, i, type_length, channel, supported = 1;
  char *response;
  // struct ussp_operation op;

  if (frame->data_length > 0)
  {
    type = frame->data[0]; // only a byte long types are handled now
    // skip extra bytes
    for (i = 0; (frame->data_length > i && (frame->data[i] & EA) == 0); i++)
      ;
    i++;
    type_length = i;
    if ((type & CR_CMD) == CR_CMD)
    {
      // command not ack
      // extract frame length
      while (frame->data_length > i)
      {
        length = (length * 128) + ((frame->data[i] & 254) >> 1);
        if ((frame->data[i] & 1) == 1)
        {
          break;
        }
        i++;
      }
      i++;

      switch ((type & ~CR_CMD))
      {
      case C_CLD:
        tr_info("The mobile station requested mux-mode termination.\n");
        if (faultTolerant)
        {
          // Signal restart
          restart = 1;
        }
        else
        {
          terminate = 1;
          terminateCount = -1; // don't need to close down channels
        }
        break;
      case C_TEST:
        break;
      case C_MSC:
        if (i + 1 < frame->data_length)
        {
          channel = ((frame->data[i] & 252) >> 2);
          i++;
          signals = (frame->data[i]);
          tr_debug("Modem status command on channel %d.\n", channel);
          if ((signals & S_FC) == S_FC)
          {
            tr_debug("No frames allowed.\n");
          }
          else
          {
            tr_debug("Frames allowed.\n");
          }
          if ((signals & S_RTC) == S_RTC)
          {
            tr_debug("RTC\n");
          }
          if ((signals & S_IC) == S_IC)
          {
            tr_debug("Ring\n");
          }
          if ((signals & S_DV) == S_DV)
          {
            tr_debug("DV\n");
          }
        }
        else
        {
          tr_error("ERROR: Modem status command, but no info. i: %d, len: %d, data-len: %d\n", i, length, frame->data_length);
        }
        break;
      default:
        tr_warning("Unknown command (%d) from the control channel.\n", type);
        response = (char *)malloc(sizeof(char) * (2 + type_length));
        response[0] = C_NSC;
        // supposes that type length is less than 128
        response[1] = EA & ((127 & type_length) << 1);
        i = 2;
        while (type_length--)
        {
          response[i] = frame->data[(i - 2)];
          i++;
        }
        api_lock();
        write_frame(0, response, i, UIH);
         api_unlock();
        free(response);
        supported = 0;
        break;
      }

      if (supported)
      {
        // acknowledge the command
        frame->data[0] = frame->data[0] & ~CR_CMD;
        api_lock();
        write_frame(0, frame->data, frame->data_length, UIH);
        api_unlock();
      }
    }
    else
    {
      // received ack for a command
      if (COMMAND_IS(C_NSC, type))
      {
        tr_warning("The mobile station didn't support the command sent.\n");
      }
      else
      {
        tr_debug("Command acknowledged by the mobile station.\n");
      }
    }
  }
}

/* Extracts and handles frames from the receiver buffer.
*
* PARAMS:
* buf - the receiver buffer
*/
int CMUXClass::extract_frames(GSM0710_Buffer *buf) // VA NNELLA READ
{
  // version test for Siemens terminals to enable version 2 functions
  const char version_test[] = "\x23\x21\x04TEMUXVERSION2\0\0";
  int framesExtracted = 0;

  GSM0710_Frame *frame;

  while ((frame = gsm0710_buffer_get_frame(buf)))
  {
    ++framesExtracted;
      if ((FRAME_IS(UI, frame) || FRAME_IS(UIH, frame)))
    {
      tr_debug("is (FRAME_IS(UI, frame) || FRAME_IS(UIH, frame))\n");
      if (frame->channel > 0)
      {
        // data from logical channel
        ussp_send_data(frame->data, frame->data_length, frame->channel);
      }
      else
      {
        // control channel command
        tr_debug("control channel command\n");
        handle_command(frame);
      }
    }
    else
    {
      // not an information frame
      tr_debug("not an information frame\n");
      switch ((frame->control & ~PF))
      {
      case UA:
        tr_debug("is FRAME_IS(UA, frame)\n");
        if (cstatus[frame->channel].opened == 1)
        {
          cstatus[frame->channel].opened = 0;
        }
        else
        {
          cstatus[frame->channel].opened = 1;
          if (frame->channel == 0)
          {
            tr_info("Control channel opened.\n");
            // send version Siemens version test
            api_lock();
            write_frame(0, version_test, 18, UIH);
            api_unlock();
          }
          else
          {
            tr_info("Logical channel %d opened.\n", frame->channel);
          }
        }
        break;
      case DM:
        if (cstatus[frame->channel].opened)
        {
          tr_info("DM received, so the channel %d was already closed.\n", frame->channel);
          cstatus[frame->channel].opened = 0;
        }
        else
        {
          if (frame->channel == 0)
          {
            tr_info("Couldn't open control channel.\n->Terminating.\n");
            return 0; // don't need to close channels
          }
          else
          {
            tr_info("Logical channel %d couldn't be opened.\n", frame->channel);
          }
        }
        break;
      case DISC:
        if (cstatus[frame->channel].opened)
        {
          cstatus[frame->channel].opened = 0;
          api_lock();
          write_frame(frame->channel, NULL, 0, UA | PF);
          api_unlock();
          if (frame->channel == 0)
          {
            tr_info("Control channel closed.\n");
            return 0; // don't need to close channels
          }
          else
          {
            tr_info("Logical channel %d closed.\n", frame->channel);
          }
        }
        else
        {
          // channel already closed
          tr_info("Received DISC even though channel %d was already closed.\n", frame->channel);
          api_lock();
          write_frame(frame->channel, NULL, 0, DM | PF);
          api_unlock();
        }
        break;
      case SABM:
        // channel open request
        if (cstatus[frame->channel].opened == 0)
        {
          if (frame->channel == 0)
          {
            tr_info("Control channel opened.\n");
          }
          else
          {
            tr_info("Logical channel %d opened.\n", frame->channel);
          }
        }
        else
        {
          // channel already opened
          tr_info("Received SABM even though channel %d was already closed.\n", frame->channel);
        }
        cstatus[frame->channel].opened = 1;
        api_lock();
        write_frame(frame->channel, NULL, 0, UA | PF);
        api_unlock();
        break;
      }
    }
    destroy_frame(frame);
  }
  return framesExtracted;
}

}
