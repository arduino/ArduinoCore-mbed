/*
 * This example shows how to use the Nicla Sense ME library to charge a battery.
 * 
 * The LED colors will change depending on the battery's operating status:
 * - Blue: Ready
 * - Yellow: Charging
 * - Green: Charging complete
 * - Red: Error
 * 
 * Instructions:
 * 1. Connect a battery to the board.
 * 2. Configure the charge current in the setup() function.
 * 3. Upload this sketch to your Nicla Sense ME board.
 * 
 * Initial author: Sebastian Romero @sebromero
 */

#include "Nicla_System.h"

constexpr auto printInterval { 10000ul };
float voltage = -1.0f;

void setup(){
  Serial.begin(115200);
  for (const auto timeout = millis() + 2500; millis() < timeout && !Serial; delay(250));
  nicla::begin(); // Initialise library
  nicla::leds.begin(); // Start I2C connection to LED driver
  nicla::setBatteryNTCEnabled(true); // Set to false if your battery doesn't have a NTC.
  
  /* 
  A safe default charging current value that works for most common LiPo batteries is 0.5C, 
  which means charging at a rate equal to half of the battery's capacity.
  For example, a 200mAh battery could be charged at 100mA (0.1A). 
  */
  nicla::enableCharging(100);
  nicla::leds.setColor(blue);
}

void loop(){

  static auto updateTimestamp = millis();

  if (millis() - updateTimestamp >= printInterval) {
    updateTimestamp = millis();

    float currentVoltage = nicla::getCurrentBatteryVoltage();
    if(currentVoltage != voltage){
      voltage = currentVoltage;    
      Serial.print("\nVoltage: ");
      Serial.println(voltage);      
    } else {
      Serial.print(".");
    }

    auto operatingStatus = nicla::getOperatingStatus();

    switch(operatingStatus) {
      case OperatingStatus::Charging:
      nicla::leds.setColor(255,100,0); // Yellow
        break;
      case OperatingStatus::ChargingComplete:
        nicla::leds.setColor(green);
        
        // This will stop further charging until enableCharging() is called again.
        nicla::disableCharging();
        break;
      case OperatingStatus::Error:
        nicla::leds.setColor(red);
        break;
      case OperatingStatus::Ready:
        nicla::leds.setColor(blue);
        break;
      default:
        nicla::leds.setColor(off);
        break;
    }

  }
}
