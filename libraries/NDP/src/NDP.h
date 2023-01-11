/*
 * Copyright (c) 2022 Arduino SA.  All rights reserved.
 * 
 * This software is available to you under a choice of one of two licenses.
 * You may choose to be licensed under the terms of the GNU General Public
 * License (GPL) Version 2, available from the file LICENSE in the main
 * directory of this source tree, or the OpenIB.org BSD license below.  Any
 * code involving Linux software will require selection of the GNU General
 * Public License (GPL) Version 2.
 * 
 * OPENIB.ORG BSD LICENSE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NDP_H
#define NDP_H
#pragma once

#include "SPI.h"
#include "Nicla_System.h"

#include <SPI.h>
#include <Wire.h>

#include "SPIFBlockDevice.h"
#include "LittleFileSystem.h"
#include "syntiant_ndp120_tiny.h"
#include "syntiant_tiny_cspi.h"


class NDPClass
{
public:
   NDPClass() {
      memset(&iif, 0, sizeof(iif));
   }

   // if no fw is passed, everything matching synpkg in the root folder will be loaded 
   int begin(const char* fw1);

   // pass a user-defined function to be called every time an interrupt
   // from the ndp is received
   void onMatch(mbed::Callback<void(char*)> _cb) {
      on_match_cb_s = _cb;
   }

   void onMatch(mbed::Callback<void(int)> _cb) {
      on_match_cb_i = _cb;
   }

   void onEvent(mbed::Callback<void(void)> _cb) {
      on_event_cb = _cb;
   }

   void onError(mbed::Callback<void(void)> _cb) {
      on_error_cb = _cb;
   }

   char** getLabels() {
      return labels;
   }

   int load(const char *fw, int bootloader = 0);
   int turnOnMicrophone();
   int configureClock();
   int getInfo();
   int turnOffMicrophone();
   int getDebugInfo();
   int configureInferenceThreshold(int threshold);
   int sendData(uint8_t *data, unsigned int len);

   void interrupts() {
      enable_interrupts(true);
   }

   void noInterrupts() {
      enable_interrupts(false);
   }

   // Check for a match event.
   // Returns:
   //   0 - no match
   //   0 < - matched ID n + 1
   int poll(void);

   // Extract data from the holding tank.
   int extractData(uint8_t *data, unsigned int *len);


   // passed to uilib, used for all SPI transfers such as in loadLog and poll
   static int spiTransfer(void *d, int mcu, uint32_t address, void *_out,
                        void *_in, unsigned int count);

   void setSpiSpeed(uint32_t speed) {
      spi_speed_general = speed;
   }

   int read(uint32_t address, void *value);

   static int sync(void *d);
   static int unsync(void *d);
   static int mbwait(void *d);
   static int get_type(void *d, unsigned int *type);

   int getAudioChunkSize(void);

public:
   static const PinName NDP_CS = p31;
   static const PinName FLASH_CS = p26;
   static const PinName NDP_INT = p14;
   static const PinName PORSTB = p18;

   // SPI speed
   static uint32_t spi_speed_general;
   static uint32_t spi_speed_initial;
   static int pdm_clk_init;

   // sensor related
   int sensorBMI270Read(int reg, int len, uint8_t data_return_array[]);
   int sensorBMI270Write(int reg, int len, uint8_t data_array[]);
   int sensorBMI270Write(int reg, int data);

   int sensorBMM150Read(int reg, int len, uint8_t data_return_array[]);
   int sensorBMM150Write(int reg, int len, uint8_t data_array[]);
   int sensorBMM150Write(int reg, int data);
private:
   mbed::Callback<void(char*)> on_match_cb_s;
   mbed::Callback<void(int)> on_match_cb_i;
   mbed::Callback<void(void)> on_event_cb;
   mbed::Callback<void(void)> on_error_cb;

   void interrupt_handler();
   int enable_interrupts(bool on);
   int checkMB();

   bool _initialized = false;
   bool _int_pin_enabled = false;

   static const size_t LABELS_STRING_LEN = 512;
   static const size_t MAX_LABELS = 32;
   static const int mbwait_timeout = 2000; /* in msec */

   static const int BMI270_SSB = 0;
   static const int BM150_SSB = 1;

   struct syntiant_ndp120_tiny_integration_interfaces_s iif;
   char label_data[LABELS_STRING_LEN] = "";
   char *labels[MAX_LABELS];

   unsigned int audio_sample_chunk_size = 0;
};

extern NDPClass NDP;

#endif
