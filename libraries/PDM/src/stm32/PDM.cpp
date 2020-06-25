/*
  PDM.cpp - library to interface with STM32 PDM microphones
  Part of Arduino - http://www.arduino.cc/

  Copyright (c) 2020 Arduino SA

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General
  Public License along with this library; if not, write to the
  Free Software Foundation, Inc., 59 Temple Place, Suite 330,
  Boston, MA  02111-1307  USA
*/

#ifdef TARGET_STM

#include "PDM.h"
#include "audio.h"
#include "mbed.h"

#define AUDIO_FREQUENCY           BSP_AUDIO_FREQUENCY_16K
#define AUDIO_IN_PDM_BUFFER_SIZE  (uint32_t)(128)

//ALIGN_32BYTES (uint16_t recordPDMBuf[AUDIO_IN_PDM_BUFFER_SIZE]) __attribute__((section(".OPEN_AMP_SHMEM")));
// FIXME: need to add an entry for RAM_D3 to linker script
uint16_t* recordPDMBuf = (uint16_t*)0x38000000;

PDMClass::PDMClass(int dinPin, int clkPin, int pwrPin) :
  _dinPin(dinPin),
  _clkPin(clkPin),
  _pwrPin(pwrPin),
  _onReceive(NULL)
{
}

PDMClass::~PDMClass()
{
}

int PDMClass::begin(int channels, long sampleRate) {

  _channels = channels;

  // fixme: only works in stereo mode
  channels = 2;

  setBufferSize(AUDIO_IN_PDM_BUFFER_SIZE / 4 * channels);

  if (isBoardRev2()) {
    mbed::I2C i2c(PB_7, PB_6);
    char data[2];

    // SW2 to 3.3V (SW2_VOLT)
    data[0] = 0x3B;
    data[1] = 0xF;
    i2c.write(8 << 1, data, sizeof(data));

    // SW1 to 3.0V (SW1_VOLT)
    data[0] = 0x35;
    data[1] = 0xF;
    i2c.write(8 << 1, data, sizeof(data));
  }

  BSP_AUDIO_IN_SelectInterface(AUDIO_IN_INTERFACE_PDM);

  /* Initialize audio IN at REC_FREQ*/
  if (BSP_AUDIO_IN_InitEx(INPUT_DEVICE_DIGITAL_MIC, AUDIO_FREQUENCY, DEFAULT_AUDIO_IN_BIT_RESOLUTION, channels) != AUDIO_OK)
  {
    return 0;
  }

  /* Start the record */
  BSP_AUDIO_IN_Record((uint16_t*)recordPDMBuf, AUDIO_IN_PDM_BUFFER_SIZE * channels);
  return 1;
}

void PDMClass::end()
{
}

int PDMClass::available()
{
  size_t avail = _doubleBuffer.available();
  if (_channels == 1) {
    return avail/2;
  } else {
    return avail;
  }
}

int PDMClass::read(void* buffer, size_t size)
{
  if (_channels == 1) {
    uint16_t temp[size*2];
    int read = _doubleBuffer.read(temp, size*2);
    for (int i = 0; i < size; i++) {
      ((uint16_t*)buffer)[i] = temp[i*2];
    }
    return read;
  }
  int read = _doubleBuffer.read(buffer, size);
  return read;
}

void PDMClass::onReceive(void(*function)(void))
{
  _onReceive = function;
}

void PDMClass::setGain(int gain)
{

}

void PDMClass::setBufferSize(int bufferSize)
{
  _doubleBuffer.setSize(bufferSize);
}

void PDMClass::IrqHandler(bool halftranfer)
{

  int start = halftranfer ? 0 : AUDIO_IN_PDM_BUFFER_SIZE;

  if (BSP_AUDIO_IN_GetInterface() == AUDIO_IN_INTERFACE_PDM && _doubleBuffer.available() == 0) {

    /* Invalidate Data Cache to get the updated content of the SRAM*/
    SCB_InvalidateDCache_by_Addr((uint32_t *)&recordPDMBuf[start], AUDIO_IN_PDM_BUFFER_SIZE * 2);

    //memcpy((uint16_t*)_doubleBuffer.data(), (uint16_t*)&recordPDMBuf[start], AUDIO_IN_PDM_BUFFER_SIZE/2);
    BSP_AUDIO_IN_PDMToPCM((uint16_t*)&recordPDMBuf[start], (uint16_t*)_doubleBuffer.data());

    /* Clean Data Cache to update the content of the SRAM */
    SCB_CleanDCache_by_Addr((uint32_t*)_doubleBuffer.data(), AUDIO_IN_PDM_BUFFER_SIZE * 2);

    _doubleBuffer.swap(_doubleBuffer.availableForWrite());
  }
  if (_onReceive) {
      _onReceive();
  }
}

extern "C" {
  /**
      @brief Calculates the remaining file size and new position of the pointer.
      @param  None
      @retval None
  */
  __attribute__((__used__)) void BSP_AUDIO_IN_TransferComplete_CallBack(void)
  {
    PDM.IrqHandler(false);
  }

  /**
      @brief  Manages the DMA Half Transfer complete interrupt.
      @param  None
      @retval None
  */
  __attribute__((__used__)) void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
  {
    PDM.IrqHandler(true);
  }
}

PDMClass PDM(0, 0, 0);

#endif