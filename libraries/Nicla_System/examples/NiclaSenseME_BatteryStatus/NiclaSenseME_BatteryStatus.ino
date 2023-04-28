#include "Nicla_System.h"
#include <ArduinoBLE.h>

constexpr auto printInterval { 4000ul };
constexpr auto batteryMeasureInterval { 5000ul };
int8_t batteryChargeLevel = -1;
int8_t batteryPercentage = -1;
float batteryVoltage = -1.0f;


#define DEVICE_NAME "NiclaSenseME"
#define DEVICE_UUID(val) ("19b10000-" val "-537e-4f6c-d104768a1214")
BLEService service(DEVICE_UUID("0000"));

BLEIntCharacteristic batteryPercentageCharacteristic(DEVICE_UUID("1001"), BLERead | BLENotify);
BLEFloatCharacteristic batteryVoltageCharacteristic(DEVICE_UUID("1002"), BLERead | BLENotify);
BLEIntCharacteristic batteryChargeLevelCharacteristic(DEVICE_UUID("1003"), BLERead | BLENotify);

bool updateBatteryLevel(bool enforceNewReading = false) {
  static auto updateTimestamp = millis();
  bool intervalFired = millis() - updateTimestamp >= batteryMeasureInterval;
  bool isFirstReading = batteryPercentage == -1 || batteryVoltage == -1.0f;

  if (intervalFired || isFirstReading || enforceNewReading) {
    Serial.println("Checking the battery level...");
    updateTimestamp = millis();
    auto percentage = nicla::getBatteryPercentage();

    if (percentage < 0) {      
      return false; // Percentage couldn't be determined.
    }

    if (batteryPercentage != percentage) {
      batteryPercentage = percentage;
      batteryVoltage = nicla::getCurrentBatteryVoltage();
      batteryChargeLevel = nicla::getBatteryChargeLevel();

      Serial.print("New battery level: ");
      Serial.println(batteryPercentage);

      return true;
    }
  }

  return false;
}

String getBatteryTemperatureDescription(int status) {
  switch (status) {
    case BATTERY_TEMPERATURE_NORMAL:
      return "Normal";
    case BATTERY_TEMPERATURE_EXTREME:
      return "Extreme";
    case BATTERY_TEMPERTURE_COOL:
      return "Cool";
    case BATTERY_TEMPERTURE_WARM:
      return "Warm";
    default:
      return "Unknown";
  }
}

String getBatteryChargeLevelDescription(int status) {
  switch (status) {
    case BATTERY_EMPTY:
      return "Empty";
    case BATTERY_ALMOST_EMPTY:
      return "Almost Empty";
    case BATTERY_HALF:
      return "Half Full";
    case BATTERY_ALMOST_FULL:
      return "Almost Full";
    case BATTERY_FULL:
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
  setupBLE();

  nicla::leds.setColor(green);
}

void loop()
{
  //BLE.poll(); // Implicit when calling BLE.connected(). Uncomment when only using BLERead

  if (BLE.connected()) {
    bool newBatteryLevelAvailable = updateBatteryLevel();

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
    
    return;
  }

  static auto updateTimestamp = millis();

  if (millis() - updateTimestamp >= printInterval) {
    updateTimestamp = millis();

    float voltage = nicla::getCurrentBatteryVoltage();
    Serial.print("Voltage: ");
    Serial.println(voltage);

    Serial.print("Battery Percent: ");
    auto percent = nicla::getBatteryPercentage();
    Serial.println(percent);

    Serial.print("Battery Temperature: ");
    auto temperature = nicla::getBatteryTemperature();
    Serial.println(getBatteryTemperatureDescription(temperature));

    auto chargeLevel = nicla::getBatteryChargeLevel();
    Serial.println("Battery is " + getBatteryChargeLevelDescription(chargeLevel));
    Serial.println("----------------------");
  }
}
