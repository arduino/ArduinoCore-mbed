#include "NDP.h"
#include <ArduinoBLE.h>

// BLE Battery Service
BLEService alertService("1802"); // Immediate alert

// BLE Battery Level Characteristic
BLEUnsignedCharCharacteristic alertLevel("2A06", BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes

const bool lowestPower = true;

void alertViaBLE(int index) {
  // notify that we recognized a keyword
  alertLevel.writeValue(2);
  delay(1000);
  alertLevel.writeValue(0);
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

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("BLExaDemo");
  BLE.setAdvertisedService(alertService); // add the service UUID
  alertService.addCharacteristic(alertLevel); // add the alert level characteristic
  BLE.addService(alertService); // Add the alert service
  alertLevel.writeValue(0); // set initial value for this characteristic

  NDP.onError(ledRedBlink);
  NDP.onMatch(alertViaBLE);
  NDP.onEvent(ledGreenOn);
  NDP.begin("mcu_fw_120_v91.synpkg");
  NDP.load("dsp_firmware_v91.synpkg");
  NDP.load("alexa_334_NDP120_B0_v11_v91.synpkg");
  NDP.turnOnMicrophone();
  NDP.interrupts();

  // start advertising
  BLE.advertise();

  // For maximum low power; please note that it's impossible to print afer calling these functions
  nicla::leds.end();
  if (lowestPower) {
    NRF_UART0->ENABLE = 0;
  }
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    // serve the updates in the interrupt
    while (central.connected()) {
      // sleep and save power
      delay(1000);
    }
  } else {
    delay(1000);
  }
}
