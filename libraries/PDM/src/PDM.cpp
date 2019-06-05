#include "PDM.h"

PDMClass::PDMClass(int dinPin, int clkPin, int pwrPin) :
  _dinPin(dinPin),
  _clkPin(clkPin),
  _pwrPin(pwrPin),
  _bufferSize(DEFAULT_PDM_BUFFER_SIZE),
  _writeBufferIndex(-1),
  _readBufferIndex(-1),
  _readIndex(0),
  _onReceive(NULL)
{
  for (int i = 0; i < NUM_PDM_BUFFERS; i++) {
    _buffer[i] = NULL;
  }
}

PDMClass::~PDMClass()
{
  for (int i = 0; i < NUM_PDM_BUFFERS; i++) {
    if (_buffer[i] != NULL) {
      free(_buffer[i]);
    }
  }
}

#define NRF_PDM_FREQ_1280K  (nrf_pdm_freq_t)(0x0A000000UL)               ///< PDM_CLK= 1.280 MHz (32 MHz / 25) => Fs= 20000 Hz
#define NRF_PDM_FREQ_2000K  (nrf_pdm_freq_t)(0x10000000UL)               ///< PDM_CLK= 2.000 MHz (32 MHz / 16) => Fs= 31250 Hz
#define NRF_PDM_FREQ_2667K  (nrf_pdm_freq_t)(0x15000000UL)               ///< PDM_CLK= 2.667 MHz (32 MHz / 12) => Fs= 41667 Hz
#define NRF_PDM_FREQ_3200K  (nrf_pdm_freq_t)(0x19000000UL)               ///< PDM_CLK= 3.200 MHz (32 MHz / 10) => Fs= 50000 Hz
#define NRF_PDM_FREQ_4000K  (nrf_pdm_freq_t)(0x20000000UL)               ///< PDM_CLK= 4.000 MHz (32 MHz /  8) => Fs= 62500 Hz


int PDMClass::begin(int channels, long sampleRate, int gain)
{
  nrfx_pdm_config_t pdmConfig;

  _channels = channels;

  switch (channels) {
    case 2:
      pdmConfig.mode = NRF_PDM_MODE_STEREO;
      break;

    case 1:
      pdmConfig.mode = NRF_PDM_MODE_MONO;
      break;

    default:
      return 0; // unsupported
  }

  switch (sampleRate) {
    case 16000:
      pdmConfig.clock_freq = NRF_PDM_FREQ_1032K; // close to 16 kHz
      break;
    case 41667:
      pdmConfig.clock_freq = NRF_PDM_FREQ_2667K;
      break;
    default:
      return 0; // unsupported
  }

  pdmConfig.edge = NRF_PDM_EDGE_LEFTFALLING;
  pdmConfig.pin_clk = _clkPin;
  pdmConfig.pin_din = _dinPin;
  pdmConfig.gain_l = gain;
  pdmConfig.gain_r = gain;
  pdmConfig.interrupt_priority = 7;

  if (_pwrPin > -1) {
    pinMode(_pwrPin, OUTPUT);
    digitalWrite(_pwrPin, HIGH);
  }

  for (int i = 0; i < NUM_PDM_BUFFERS; i++) {
    _buffer[i] = (int16_t*)realloc(_buffer[i], _bufferSize);
  }

  _writeBufferIndex = -1;
  _readBufferIndex = -1;

  if (nrfx_pdm_init(&pdmConfig, (nrfx_pdm_event_handler_t)PDMClass::onPdmEvent) != NRFX_SUCCESS) {
    return 0;
  }

  if (nrfx_pdm_start() != NRFX_SUCCESS) {
    return 0;
  }

  return 1;
}

void PDMClass::end()
{
  if (_pwrPin > -1) {
    digitalWrite(_pwrPin, LOW);
    pinMode(_pwrPin, INPUT);
  }

  nrfx_pdm_stop();
  nrfx_pdm_uninit();
}

int PDMClass::available()
{
  if (_readBufferIndex != -1) {
    return (_bufferSize - _readIndex);
  } else {
    return 0;
  }
}

int PDMClass::read(void* buffer, size_t size)
{
  int avail = available();

  if (size > (size_t)avail) {
    size = avail;
  }

  int16_t* in = &_buffer[_readBufferIndex][_readIndex];
  int16_t* out = (int16_t*)buffer;

  memcpy(out, in, size);
  _readIndex += (size / 2);

  return size;
}

void PDMClass::onReceive(void(*function)(void))
{
  _onReceive = function;
}

void PDMClass::onReceive(void(*function)(void* buf, size_t size))
{
  _onReceiveParams = function;
}

void PDMClass::setBufferSize(int bufferSize)
{
  _bufferSize = bufferSize;
}

void PDMClass::onPdmEvent(nrfx_pdm_evt_t const* const evt)
{
  PDM.handlePdmEvent(evt);
}

void PDMClass::handlePdmEvent(nrfx_pdm_evt_t const* const evt)
{
  if (evt->error != NRFX_PDM_NO_ERROR) {
    return; // error -> ignore
  }

  if (evt->buffer_requested) {
    // start the next transfer
    _writeBufferIndex = (_writeBufferIndex + 1) % NUM_PDM_BUFFERS;

    nrfx_pdm_buffer_set(_buffer[_writeBufferIndex], _bufferSize / 2);
  }

  if (evt->buffer_released) {
    for (int i = 0; i < NUM_PDM_BUFFERS; i++) {
      if (evt->buffer_released == _buffer[i]) {
        _readBufferIndex = i;
        break;
      }
    }

    _readIndex = 0;

    if (_onReceive) {
      _onReceive();
    }
    if (_onReceiveParams) {
      _onReceiveParams(_buffer[_readBufferIndex], _bufferSize);
    }
  }
}

PDMClass PDM(PIN_PDM_DIN, PIN_PDM_CLK, PIN_PDM_PWR);
