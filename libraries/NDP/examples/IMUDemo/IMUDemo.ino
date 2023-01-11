#include "NDP.h"

//const bool lowestPower = true;
const bool lowestPower = false;

void ledBlueOn(char* label) {
  nicla::leds.begin();
  nicla::leds.setColor(blue);
  delay(200);
  nicla::leds.setColor(off);
  if (!lowestPower) {
    Serial.println(label);
  }
  nicla::leds.end();
}

void ledGreenOn() {
  nicla::leds.begin();
  nicla::leds.setColor(green);
  delay(200);
  nicla::leds.setColor(off);
  nicla::leds.end();
}

void ledRedBlink() {
  while (1) {
    nicla::leds.begin();
    nicla::leds.setColor(red);
    delay(200);
    nicla::leds.setColor(off);
    delay(200);
    nicla::leds.end();
  }
}

void setup() {

  Serial.begin(115200);
  nicla::begin();
  nicla::disableLDO();
  nicla::leds.begin();

  NDP.onError(ledRedBlink);
  NDP.onMatch(ledBlueOn);
  NDP.onEvent(ledGreenOn);
  Serial.println("Loading synpackages");
  NDP.begin("mcu_fw_120_v91.synpkg");
  NDP.load("dsp_firmware_v91.synpkg");
  NDP.load("ei_model_imu.synpkg");
  Serial.println("packages loaded");
  NDP.getInfo();
  NDP.configureInferenceThreshold(1088);
  NDP.interrupts();

  // For maximum low power; please note that it's impossible to print after calling these functions
  nicla::leds.end();
  if (lowestPower) {
    NRF_UART0->ENABLE = 0;
  }
}

extern "C" const unsigned char data_opensset_bin[];
extern "C" const unsigned char data_circ_bin[];
extern "C" const unsigned int data_opensset_bin_len;
extern "C" const unsigned int data_circ_bin_len;


void loop() {
  Serial.println("Sending openset data... (no match expected)");
  NDP.sendData((uint8_t*)data_opensset_bin, data_opensset_bin_len);
  delay(1000);

  Serial.println("Sending circular IMU data.... (match expected)");
  NDP.sendData((uint8_t*)data_circ_bin, data_circ_bin_len);
  delay(5000);
}