/*
  Stream the microphone audio to serial port
  The file is compressed using G722 codec
  Prerequisite libraries:
    https://github.com/pschatzmann/arduino-libg722
    https://github.com/pschatzmann/arduino-audio-tools/

  Precedure to extract audio:
  Setup the serial port as raw, for example with
     stty -F /dev/ttyACM0 115200 raw
  Dump the data
     cat /dev/ttyACM0 > test.g722
  Open Audacity
     audacity test.g722
*/

#include "Arduino.h"
#include "NDP.h"

#undef abs
#define USE_INT24_FROM_INT
#include "AudioTools.h"
#include "AudioCodecs/CodecG722.h"

G722Encoder encoder;

uint8_t data[2048];

void ledGreenOn() {
  nicla::leds.begin();
  nicla::leds.setColor(green);
  delay(200);
  nicla::leds.setColor(off);
  nicla::leds.end();
}

void setup() {

  Serial.begin(115200);
  nicla::begin();
  nicla::disableLDO();
  nicla::leds.begin();

  NDP.onEvent(ledGreenOn);

  AudioBaseInfo bi;
  bi.channels = 1;
  bi.sample_rate = 16000;

  encoder.setOptions(0);
  encoder.begin(bi);

  encoder.setOutputStream(Serial);

  NDP.begin("mcu_fw_120_v91.synpkg");
  NDP.load("dsp_firmware_v91.synpkg");
  NDP.load("alexa_334_NDP120_B0_v11_v91.synpkg");
  NDP.turnOnMicrophone();
  int chunk_size = NDP.getAudioChunkSize();
  if (chunk_size >= sizeof(data)) {
    for(;;);
  }
}

void loop() {
  unsigned int len = 0;

  NDP.extractData(data, &len);
  encoder.write(data, len);
}
