#if defined(ARDUINO_ARCH_RP2040)

#include "Arduino.h"
#include "PDM.h"
#include "OpenPDMFilter.h"
#include "mbed_interface.h"

extern "C" {
#include "hardware/pio.h"
#include "hardware/dma.h"
#include "hardware/clocks.h"
}

#include "pdm.pio.h"

// Hardware peripherals used
uint dmaChannel = 0;
PIO pio = pio0;
uint sm = 0;

// PIO program offset
static uint offset;

// raw buffers contain PDM data
#define RAW_BUFFER_SIZE 512 // should be a multiple of (decimation / 8)
uint8_t rawBuffer0[RAW_BUFFER_SIZE];
uint8_t rawBuffer1[RAW_BUFFER_SIZE];
uint8_t* rawBuffer[2] = {rawBuffer0, rawBuffer1};
volatile int rawBufferIndex = 0; 

int decimation = 128;

// final buffer is the one to be filled with PCM data
int16_t* volatile finalBuffer;

// OpenPDM filter used to convert PDM into PCM
#define FILTER_GAIN     16
TPDMFilter_InitStruct filter;

extern "C" {
  __attribute__((__used__)) void dmaHandler(void)
  {
    PDM.IrqHandler(true);
  }
}


PDMClass::PDMClass(int dinPin, int clkPin, int pwrPin) :
  _dinPin(dinPin),
  _clkPin(clkPin),
  _pwrPin(pwrPin),
  _onReceive(NULL),
  _gain(-1),
  _channels(-1),
  _samplerate(-1),
  _init(-1),
  _cutSamples(100)
{
}

PDMClass::~PDMClass()
{
}

int PDMClass::begin(int channels, int sampleRate)
{
  //_channels = channels; // only one channel available

  // clear the final buffers
  _doubleBuffer.reset();
  finalBuffer = (int16_t*)_doubleBuffer.data();
  int finalBufferLength = _doubleBuffer.availableForWrite() / sizeof(int16_t);
  _doubleBuffer.swap(0);

  // The mic accepts an input clock from 1.2 to 3.25 Mhz
  // Setup the decimation factor accordingly
  if ((sampleRate * decimation * 2) > 3250000) {
    decimation = 64;
  }

  // Sanity check, abort if still over 3.25Mhz
  if ((sampleRate * decimation * 2) > 3250000) {
    mbed_error_printf("Sample rate too high, the mic would glitch\n");
    mbed_die();
  }

  int rawBufferLength = RAW_BUFFER_SIZE / (decimation / 8);
  // Saturate number of samples. Remaining bytes are dropped.
  if (rawBufferLength > finalBufferLength) {
    rawBufferLength = finalBufferLength;
  }

  /* Initialize Open PDM library */
  filter.Fs = sampleRate;
  filter.MaxVolume = 1;
  filter.nSamples = rawBufferLength;
  filter.LP_HZ = sampleRate/2;
  filter.HP_HZ = 10;
  filter.In_MicChannels = 1;
  filter.Out_MicChannels = 1;
  filter.Decimation = decimation;
  if(_gain == -1) {
    _gain = FILTER_GAIN;
  }
  filter.filterGain = _gain;
  Open_PDM_Filter_Init(&filter);

  // Configure PIO state machine
  float clkDiv = (float)clock_get_hz(clk_sys) / sampleRate / decimation / 2; 
  if(pio_can_add_program(pio, &pdm_pio_program)) {
    offset = pio_add_program(pio, &pdm_pio_program);
    pdm_pio_program_init(pio, sm, offset, _clkPin, _dinPin, clkDiv);
  } else {
    mbed_error_printf("Cannot load pio program\n");
    mbed_die();
  }

  // Wait for microphone 
  delay(100);

  // Configure DMA for transferring PIO rx buffer to raw buffers
  dma_channel_config c = dma_channel_get_default_config(dmaChannel);
  channel_config_set_read_increment(&c, false);
  channel_config_set_write_increment(&c, true);
  channel_config_set_dreq(&c, pio_get_dreq(pio, sm, false));
  channel_config_set_transfer_data_size(&c, DMA_SIZE_8);

  // Clear DMA interrupts
  dma_hw->ints0 = 1u << dmaChannel; 
  // Enable DMA interrupts
  dma_channel_set_irq0_enabled(dmaChannel, true);
  irq_set_exclusive_handler(DMA_IRQ_0, dmaHandler);
  irq_set_enabled(DMA_IRQ_0, true);

  dma_channel_configure(dmaChannel, &c,
    rawBuffer[rawBufferIndex],        // Destinatinon pointer
    &pio->rxf[sm],      // Source pointer
    RAW_BUFFER_SIZE, // Number of transfers
    true                // Start immediately
  );

  _cutSamples = 100;

  _init = 1;

  return 1;
}

void PDMClass::end()
{
  NVIC_DisableIRQ(DMA_IRQ_0n);
  pio_remove_program(pio, &pdm_pio_program, offset);
  dma_channel_abort(dmaChannel);
  pinMode(_clkPin, INPUT);
  decimation = 128;
  rawBufferIndex = 0;
  offset = 0;
}

int PDMClass::available()
{
  NVIC_DisableIRQ(DMA_IRQ_0n);
  size_t avail = _doubleBuffer.available();
  NVIC_EnableIRQ(DMA_IRQ_0n);
  return avail;
}

int PDMClass::read(void* buffer, size_t size)
{
  NVIC_DisableIRQ(DMA_IRQ_0n);
  int read = _doubleBuffer.read(buffer, size);
  NVIC_EnableIRQ(DMA_IRQ_0n);
  return read;
}

void PDMClass::onReceive(void(*function)(void))
{
  _onReceive = function;
}

void PDMClass::setGain(int gain)
{
  _gain = gain;
  if(_init == 1) {
    filter.filterGain = _gain;
    Open_PDM_Filter_Init(&filter);
  }
}

void PDMClass::setBufferSize(int bufferSize)
{
  _doubleBuffer.setSize(bufferSize);
}

void PDMClass::IrqHandler(bool halftranfer)
{
  // Clear the interrupt request.
  dma_hw->ints0 = 1u << dmaChannel; 
  // Restart dma pointing to the other buffer 
  int shadowIndex = rawBufferIndex ^ 1;
  dma_channel_set_write_addr(dmaChannel, rawBuffer[shadowIndex], true);

  if (_doubleBuffer.available()) {
    // buffer overflow, stop
    return end();
  }

  // fill final buffer with PCM samples
  if (filter.Decimation == 128) {
    Open_PDM_Filter_128(rawBuffer[rawBufferIndex], finalBuffer, 1, &filter);
  } else {
    Open_PDM_Filter_64(rawBuffer[rawBufferIndex], finalBuffer, 1, &filter);
  }

  if (_cutSamples) {
    memset(finalBuffer, 0, _cutSamples);
    _cutSamples = 0;
  }

  // swap final buffer and raw buffers' indexes
  finalBuffer = (int16_t*)_doubleBuffer.data();
  _doubleBuffer.swap(filter.nSamples * sizeof(int16_t));
  rawBufferIndex = shadowIndex;

  if (_onReceive) {
    _onReceive();
  }
}

PDMClass PDM(PIN_PDM_DIN, PIN_PDM_CLK, -1);

#endif
