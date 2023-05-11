/*
 * This example shows how to use the Nicla Sense ME library to read the battery status and send it over BLE.
 * 
 * When not connected over BLE, the battery status is printed over the serial port every 4 seconds.
 * When connected over BLE, the battery status is checked every 4 seconds and sent over BLE if a value has changed.
 * That is, when using the notification mechanism in the BatteryMonitor web app, which is the default.
 * The BatteryMonitor web app can also be configured to poll the battery status every X seconds.
 * 
 * The LED colors are used to indicate the BLE connection status:
 * - Green: Board is ready but no BLE connection has been established yet.
 * - Blue: Board is connected over BLE to a device that wants to read the battery status.
 * - Red: A device that was previously connected over BLE has disconnected.
 * 
 * Instructions:
 * 1. Upload this sketch to your Nicla Sense ME board.
 * 2. Open the BatteryMonitor web app (index.html) in a browser that supports Web Bluetooth (Chrome, Edge, Opera, etc.).
 * 3. Connect to your Nicla Sense ME board by clicking the "Connect" button.
 * 
 * Initial author: Sebastian Romero @sebromero
 */

#include "Nicla_System.h"
#include <ArduinoBLE.h>

constexpr auto printInterval { 4000ul };
constexpr auto batteryUpdateInterval { 4000ul };
int8_t batteryChargeLevel = -1;
int8_t batteryPercentage = -1;
float batteryVoltage = -1.0f;
int8_t runsOnBattery = -1; // Using an int to be able to represent an unknown state.
int8_t batteryIsCharging = -1; // Using an int to be able to represent an unknown state.


#define DEVICE_NAME "NiclaSenseME"
#define DEVICE_UUID(val) ("19b10000-" val "-537e-4f6c-d104768a1214")
BLEService service(DEVICE_UUID("0000"));

BLEIntCharacteristic batteryPercentageCharacteristic(DEVICE_UUID("1001"), BLERead | BLENotify);
BLEFloatCharacteristic batteryVoltageCharacteristic(DEVICE_UUID("1002"), BLERead | BLENotify);
BLEIntCharacteristic batteryChargeLevelCharacteristic(DEVICE_UUID("1003"), BLERead | BLENotify);
BLEBooleanCharacteristic runsOnBatteryCharacteristic(DEVICE_UUID("1004"), BLERead | BLENotify);
BLEBooleanCharacteristic isChargingCharacteristic(DEVICE_UUID("1005"), BLERead | BLENotify);

bool updateBatteryStatus(){
  static auto updateTimestamp = millis();
  bool intervalFired = millis() - updateTimestamp >= batteryUpdateInterval;
  bool isFirstReading = runsOnBattery == -1 || batteryIsCharging == -1;

  if (intervalFired || isFirstReading) {
    Serial.println("Checking the battery status...");
    updateTimestamp = millis();
    int8_t isCharging = nicla::getOperatingStatus() == OperatingStatus::Charging;
    int8_t batteryPowered = nicla::runsOnBattery();
    bool valueUpdated = false;

    if (batteryIsCharging != isCharging) {
      batteryIsCharging = isCharging;
      valueUpdated = true;
    }

    if (runsOnBattery != batteryPowered) {
      runsOnBattery = batteryPowered;
      valueUpdated = true;
    }

    return valueUpdated;
  }

  return false;
}

bool updateBatteryLevel() {
  static auto updateTimestamp = millis();
  bool intervalFired = millis() - updateTimestamp >= batteryUpdateInterval;
  bool isFirstReading = batteryPercentage == -1 || batteryVoltage == -1.0f;

  if (intervalFired || isFirstReading) {
    Serial.println("Checking the battery level...");
    updateTimestamp = millis();
    auto percentage = nicla::getBatteryVoltagePercentage();

    if (percentage < 0) {
      Serial.println("Battery voltage percentage couldn't be determined.");
      return false;
    }

    // Only if the percentage has changed, we update the values as they depend on it.
    if (batteryPercentage != percentage) {
      int8_t currentChargeLevel = static_cast<int8_t>(nicla::getBatteryChargeLevel());
      if(currentChargeLevel == 0){
        Serial.println("Battery charge level couldn't be determined.");
        return false;
      }

      auto currentVoltage = nicla::getCurrentBatteryVoltage();
      if(currentVoltage == 0){
        Serial.println("Battery voltage couldn't be determined.");
        return false;
      }

      batteryPercentage = percentage;
      batteryChargeLevel = currentChargeLevel;
      batteryVoltage = currentVoltage;
      
      return true;
    }
  }

  return false;
}

String getBatteryTemperatureDescription(BatteryTemperature status) {
  switch (status) {
    case BatteryTemperature::Normal:
      return "Normal";
    case BatteryTemperature::Extreme:
      return "Extreme";
    case BatteryTemperature::Cool:
      return "Cool";
    case BatteryTemperature::Warm:
      return "Warm";
    default:
      return "Unknown";
  }
}

String getBatteryChargeLevelDescription(BatteryChargeLevel status) {
  switch (status) {
    case BatteryChargeLevel::Empty:
      return "Empty";
    case BatteryChargeLevel::AlmostEmpty:
      return "Almost Empty";
    case BatteryChargeLevel::HalfFull:
      return "Half Full";
    case BatteryChargeLevel::AlmostFull:
      return "Almost Full";
    case BatteryChargeLevel::Full:
      return "Full";
    default:
      return "Unknown";
  }
}


void blePeripheralDisconnectHandler(BLEDevice central) {
  nicla::leds.setColor(red);
  Serial.println("Device disconnected.");
}

void blePeripheralConnectHandler(BLEDevice central) {
  nicla::leds.setColor(blue);
  Serial.println("Device connected.");
}

void onBatteryVoltageCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  Serial.println("Requesting battery voltage...");
  updateBatteryLevel();
  Serial.print("Battery voltage: ");
  Serial.println(batteryVoltage);
  batteryVoltageCharacteristic.writeValue(batteryVoltage);
}

void onBatteryPercentageCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  Serial.println("Requesting battery percentage...");
  updateBatteryLevel();
  Serial.print("Battery Percent: ");
  Serial.println(batteryPercentage);
  batteryPercentageCharacteristic.writeValue(batteryPercentage);
}

void onBatteryChargeLevelCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  Serial.println("Requesting battery charge level...");
  updateBatteryLevel();
  Serial.print("Battery Charge Level: ");
  Serial.println(batteryChargeLevel);
  batteryChargeLevelCharacteristic.writeValue(batteryChargeLevel);
}

void onRunsOnBatteryCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  Serial.println("Checking if device runs on battery...");
  updateBatteryStatus();
  Serial.print("Runs on battery: ");
  Serial.println(runsOnBattery == 1 ? "Yes" : "No");
  runsOnBatteryCharacteristic.writeValue(runsOnBattery == 1);
}

void onIsChargingCharacteristicRead(BLEDevice central, BLECharacteristic characteristic) {
  Serial.println("Checking if battery is charging...");
  updateBatteryStatus();
  Serial.print("Battery is charging: ");
  Serial.println(batteryIsCharging == 1 ? "Yes" : "No");
  isChargingCharacteristic.writeValue(batteryIsCharging == 1);
}

void onCharacteristicSubscribed(BLEDevice central, BLECharacteristic characteristic) {
  Serial.println("Device subscribed to characteristic: " + String(characteristic.uuid()));
}

void setupBLE() {
  if (!BLE.begin()) {
    Serial.println("Failed to initialized BLE!");

    while (true) {
      // Blink the red LED to indicate failure
      nicla::leds.setColor(red);
      delay(500);
      nicla::leds.setColor(off);
      delay(500);
    }
  }

  BLE.setLocalName(DEVICE_NAME);
  BLE.setDeviceName(DEVICE_NAME);
  BLE.setAdvertisedService(service);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);

  service.addCharacteristic(batteryPercentageCharacteristic);
  batteryPercentageCharacteristic.setEventHandler(BLERead, onBatteryPercentageCharacteristicRead);
  batteryPercentageCharacteristic.setEventHandler(BLESubscribed, onCharacteristicSubscribed);
  batteryPercentageCharacteristic.writeValue(batteryPercentage);

  service.addCharacteristic(batteryVoltageCharacteristic);
  batteryVoltageCharacteristic.setEventHandler(BLERead, onBatteryVoltageCharacteristicRead);
  batteryVoltageCharacteristic.setEventHandler(BLESubscribed, onCharacteristicSubscribed);
  batteryVoltageCharacteristic.writeValue(batteryVoltage);

  service.addCharacteristic(batteryChargeLevelCharacteristic);
  batteryChargeLevelCharacteristic.setEventHandler(BLERead, onBatteryChargeLevelCharacteristicRead);
  batteryChargeLevelCharacteristic.setEventHandler(BLESubscribed, onCharacteristicSubscribed);
  batteryChargeLevelCharacteristic.writeValue(batteryChargeLevel);

  service.addCharacteristic(runsOnBatteryCharacteristic);
  runsOnBatteryCharacteristic.setEventHandler(BLERead, onRunsOnBatteryCharacteristicRead);
  runsOnBatteryCharacteristic.setEventHandler(BLESubscribed, onCharacteristicSubscribed);
  runsOnBatteryCharacteristic.writeValue(runsOnBattery == 1);

  service.addCharacteristic(isChargingCharacteristic);
  isChargingCharacteristic.setEventHandler(BLERead, onIsChargingCharacteristicRead);
  isChargingCharacteristic.setEventHandler(BLESubscribed, onCharacteristicSubscribed);
  isChargingCharacteristic.writeValue(batteryIsCharging == 1);

  BLE.addService(service);
  BLE.advertise();
}

void setup()
{
  Serial.begin(115200);
  for (const auto timeout = millis() + 2500; millis() < timeout && !Serial; delay(250));
  
  // run this code once when Nicla Sense ME board turns on
  nicla::begin(); // initialise library
  nicla::leds.begin(); // Start I2C connection

  nicla::setBatteryNTCEnabled(true); // Set to false if your battery doesn't have an NTC thermistor.
  nicla::enableCharging();

  nicla::leds.setColor(green);

  setupBLE();
}

void loop()
{
  //BLE.poll(); // Implicit when calling BLE.connected(). Uncomment when only using BLERead (polling mechanism)

  // Check if a BLE device is connected and handle battery updates 
  // via the notification mechanism.
  if (BLE.connected()) {
    bool newBatteryLevelAvailable = updateBatteryLevel();
    bool newBatteryStatusAvailable = updateBatteryStatus();

    if (batteryPercentageCharacteristic.subscribed() && newBatteryLevelAvailable) {
      Serial.print("Battery Percentage: ");
      Serial.println(batteryPercentage);
      batteryPercentageCharacteristic.writeValue(batteryPercentage);
    }

    if (batteryVoltageCharacteristic.subscribed() && newBatteryLevelAvailable) {
      Serial.print("Battery Voltage: ");
      Serial.println(batteryVoltage);
      batteryVoltageCharacteristic.writeValue(batteryVoltage);
    }
    
    if (batteryChargeLevelCharacteristic.subscribed() && newBatteryLevelAvailable) {
      Serial.print("Battery charge level: ");
      Serial.println(batteryChargeLevel);
      batteryChargeLevelCharacteristic.writeValue(batteryChargeLevel);
    }

    if(runsOnBatteryCharacteristic.subscribed() && newBatteryStatusAvailable) {
      Serial.print("Runs on battery: ");
      Serial.println(runsOnBattery == 1 ? "Yes" : "No");
      runsOnBatteryCharacteristic.writeValue(runsOnBattery == 1);
    }

    if(isChargingCharacteristic.subscribed() && newBatteryStatusAvailable) {
      Serial.print("Battery is charging: ");
      Serial.println(batteryIsCharging == 1 ? "Yes" : "No");
      isChargingCharacteristic.writeValue(batteryIsCharging == 1);
    }

    return;
  }

  static auto updateTimestamp = millis();

  if (millis() - updateTimestamp >= printInterval) {
    updateTimestamp = millis();

    float voltage = nicla::getCurrentBatteryVoltage();
    Serial.print("Voltage: ");
    Serial.println(voltage);

    Serial.print("Battery Percent: ");
    auto percent = nicla::getBatteryVoltagePercentage();
    Serial.println(percent);

    Serial.print("Battery Temperature: ");
    auto temperature = nicla::getBatteryTemperature();
    Serial.println(getBatteryTemperatureDescription(temperature));

    auto chargeLevel = nicla::getBatteryChargeLevel();
    Serial.println("Battery is " + getBatteryChargeLevelDescription(chargeLevel));

    bool isCharging = nicla::getOperatingStatus() == OperatingStatus::Charging;
    Serial.print("Battery is charging: ");
    Serial.println(isCharging ? "Yes" : "No");

    bool runsOnBattery = nicla::runsOnBattery();
    Serial.print("Runs on battery: ");
    Serial.println(runsOnBattery ? "Yes" : "No");

    
    Serial.println("----------------------");
  }
}
