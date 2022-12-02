#include "NDP.h"

#include "BMI270_Init.h"

void ledBlueOn(char* label) {
  nicla::leds.begin();
  nicla::leds.setColor(blue);
  delay(200);
  nicla::leds.setColor(off);
  Serial.println(label);
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

uint8_t sensor_all_bytes[16]={0};

bool debugTrace = false;

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
  NDP.load("alexa_334_NDP120_B0_v11_v91.synpkg");
  Serial.println("packages loaded");
  NDP.getInfo();
  Serial.println("Configure clock");
  NDP.turnOnMicrophone();
  NDP.interrupts();


  // Basic master SPI controls
  ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  uint8_t s = 0; // flag for error in communication with motion sensor
  uint8_t targetDevice = 0; // "0" or "1". "0": BMI270. "1": BMM150.
  uint8_t ndpCSPISpeedFactor = 4; // this factor can take values 1, 2, 3 or 4. This is a division factor from NDP120 internal
                                  // clock. The larger the factor, the slower the CSPI speed - which should ease compatibility with slow SPI targets.
  uint32_t bmi_sensor_address = 0;
  uint16_t number_of_bytes = 0;
  uint32_t *bmi270_initialization_pointer;

  // IMPORTANT: All reads to register address in BMI270 will use "Reg4Read" for method. That's because readout
  // from BMI270 using SPI interfaace will come with 1 dummy byte pre-pended to any actual content.
  // That is, whenever a 1 byte content from BMI270 is to be read, it will come through the BMI270 SPI interface
  // prepended by 1 junk byte and 1 dummy byte, before the actual byte of interest comes.
  // BMI270 Initiliztion file to be used:
  bmi270_initialization_pointer = (uint32_t*)bmi270_maximum_fifo_config_file;
  number_of_bytes = sizeof(bmi270_maximum_fifo_config_file);

  //Reading BMI270 Chip ID (twice...to put the device in SPI mode)
  bmi_sensor_address = 0x00;
  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1, sensor_all_bytes);
  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1, sensor_all_bytes);
  Serial.print("BMI270 chip ID is (expected is 0x24): 0x");
  Serial.println(sensor_all_bytes[0], HEX);
  delay(100);

  //Initialization process. Following BMI270 initialization process: page 18/150
  // disable PWR_CONF.adv_power_save
  bmi_sensor_address = 0x7c;
  sensor_all_bytes[0] = 0x02;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);
  delay(20); //delay 20ms much longer than reqired 450us

  // prepare config load INIT_CTRL = 0x00
  bmi_sensor_address = 0x59;
  sensor_all_bytes[0] = 0x00;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);
  delay(20); //delay 20ms much longer than 450us

  // Push bmi270_maximum_fifo_config_file[] to initialize BMI270
  Serial.print("BMI270 config file has number of bytes equal to: ");
  Serial.println(number_of_bytes);

  // burst write to reg INIT_DATA Start with byte 0
  // timing the process of pushing all initialization
  Serial.print("\nStarting the upload of bmi270 config file ...");
  int initialization_process_time = 0;
  initialization_process_time = millis();
  if (debugTrace) {
    Serial.println("Writing data");
    for (int i=0; i<10; i++){
      Serial.print("file index: ");
      Serial.print(i);
      Serial.print("file value: ");
      Serial.println(bmi270_maximum_fifo_config_file[i],HEX);
    }
  }
  delay(20);

  bmi_sensor_address = 0x5e;
  s = NDP.transparentNiclaVoiceBMI270SensorDataInitialization(targetDevice, ndpCSPISpeedFactor, number_of_bytes, bmi270_maximum_fifo_config_file);
  initialization_process_time = (millis() - initialization_process_time)/1000.0;
  Serial.println("\nDone with initialization ...");
  if (debugTrace) {
    Serial.print("\nTime to upload initialization file (seconds): ");
    Serial.println(initialization_process_time);
  }
  delay(20); //delay 20ms much longer than 450us

  // complete config load
  //////////////////////////////////////////////////////////////
  bmi_sensor_address = 0x59;
  sensor_all_bytes[0] = 0x01;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);

  ///////////////////////////////////////////
  delay(20); //delay 20ms much longer than 450us
  // check initialization status
  // read INTERNAL_STATUS
  bmi_sensor_address = 0x21;
  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1, sensor_all_bytes);
  Serial.print("BMI270 Status Register at address 0x21 is (expected is 0x01): 0x");
  Serial.println(sensor_all_bytes[0], HEX);

  bmi_sensor_address = 0x59;
  sensor_all_bytes[0] = 0x00;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);

  bmi_sensor_address = 0x5b;
  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 2, sensor_all_bytes);
  if (debugTrace) {
    Serial.println("Before writing on 5b/c ");
    Serial.println(sensor_all_bytes[0], HEX);
    Serial.println(sensor_all_bytes[1], HEX);
  }
  sensor_all_bytes[0] = 0x00;
  sensor_all_bytes[1] = 0x00;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 2,  sensor_all_bytes);
  delay(20);
  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 2, sensor_all_bytes);
  if (debugTrace) {
    Serial.println("After resetting on 5b/c ");
    Serial.println(sensor_all_bytes[0], HEX);
    Serial.println(sensor_all_bytes[1], HEX);
  }

  bmi_sensor_address = 0x5e;
  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 10, sensor_all_bytes);
  if (debugTrace) {
    Serial.println("Reading data");
    for (int i = 0; i < 10; i++){
      Serial.println(sensor_all_bytes[i], HEX);
    }
  }
  bmi_sensor_address = 0x5b;
  sensor_all_bytes[0] = 0x0F;
  sensor_all_bytes[1] = 0x09;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 2,  sensor_all_bytes);
  delay(20);
  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 2, sensor_all_bytes);
  if (debugTrace) {
    Serial.println("After resetting 9F (last 10) on 5b/c ");
    Serial.println(sensor_all_bytes[0], HEX);
    Serial.println(sensor_all_bytes[1], HEX);
  }

  bmi_sensor_address = 0x5e;
  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 10, sensor_all_bytes);
  if (debugTrace) {
    Serial.println("Reading data");
    for (int i = 0; i < 10; i++){
      Serial.println(sensor_all_bytes[i], HEX);
    }
  }

  // configuring device to normal power mode with both Accelerometer and gyroscope working
  ////////////////////////////////////////////////////////////////////////////////////////
  // setting PWR_CTRL
  bmi_sensor_address = 0x7d;
  sensor_all_bytes[0] = 0x0e;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);
  delay(20); //delay 20ms much longer than 450us

  //ACC_CONF
  ////////////
  bmi_sensor_address = 0x40;
  sensor_all_bytes[0] = 0xa8;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);
  delay(20); //delay 20ms much longer than 450us

  //ACC_RANGE
  ////////////
  bmi_sensor_address = 0x41;
  sensor_all_bytes[0] = 0x00;  /* +* 2g */
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);
  delay(20); //delay 20ms much longer than 450us

  //GYR_CONF
  //////////
  bmi_sensor_address = 0x42;
  sensor_all_bytes[0] = 0xa9;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);
  delay(20); //delay 20ms much longer than 450us

  //GYR_RANGE
  //////////
  bmi_sensor_address = 0x43;
  sensor_all_bytes[0]= 0x11;  /* +-250 */
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);
  delay(20); //delay 20ms much longer than 450us

  //PWR_CONF
  ////////////
  bmi_sensor_address = 0x7c;
  sensor_all_bytes[0] = 0x02;
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 1,  sensor_all_bytes);
  delay(20); //delay 20ms much longer than 450us


  ///////////////////////////////////////////// BMM 150 test /////////////////////////////////////////////////////
  //Reading BMM 150 CHip ID
  ///////////////////////////////////////////
  uint32_t bmm_sensor_address = 0;
  targetDevice = 1; // 0, 1 possible values. 0: ST or BMI270. 1: BMM150

  bmm_sensor_address = 0x4B;
  sensor_all_bytes[0] = 0x1; // Is it equivalent to 0x01?
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmm_sensor_address, 1,  sensor_all_bytes);

  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmm_sensor_address, 1, sensor_all_bytes);
  Serial.print("BMM150 power control byte at address 0x4B is (expected is 0x01): 0x");
  Serial.println(sensor_all_bytes[0], HEX);

  bmm_sensor_address = 0x4C;
  sensor_all_bytes[0] = 0x00; // Is it in sleep mode by default?
  s = NDP.transparentNiclaVoiceSensorRegWrite(targetDevice, ndpCSPISpeedFactor, bmm_sensor_address, 1,  sensor_all_bytes);

  bmm_sensor_address = 0x40;
  s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmm_sensor_address, 1, sensor_all_bytes);
  Serial.print("BMM150 power chip ID at address 0x40 is (expected is 0x32): 0x");
  Serial.println(sensor_all_bytes[0], HEX);
}

#define NUM_LOOPS_PER_SENSOR 15
void loop() {
  uint8_t s = 0; // flag for error in communication with motion sensor
  uint32_t bmi_sensor_address = 0, bmm_sensor_address = 0;
  uint8_t targetDevice = 0; // 0 or 1. 0: BMI270. 1: BMM150
  uint8_t ndpCSPISpeedFactor = 4; // this factor can take values 1, 2, 3 or 4. This is a division factor from NDP120 reference
  static int loopCounter = 0;

  //read one accel and one gyro datum at a time
  //read acc_x_7_0 and acc_x_15_8
  int16_t x_acc, y_acc, z_acc, x_gyr, y_gyr, z_gyr  ;
  int16_t x_mag, y_mag, z_mag, hall;
  if (loopCounter < NUM_LOOPS_PER_SENSOR) {
    if (!loopCounter) {
      Serial.println("\nTesting Nicla Voice IMU Sensor");
    }
    bmi_sensor_address = 0x0C;

    s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmi_sensor_address, 16, sensor_all_bytes);
    if (debugTrace) {
      Serial.print("number of bytes outputted: ");
      Serial.println(s);
    }
    x_acc = (0x0000 | sensor_all_bytes[0] | sensor_all_bytes[1] << 8);
    y_acc = (0x0000 | sensor_all_bytes[2] | sensor_all_bytes[3] << 8);
    z_acc = (0x0000 | sensor_all_bytes[4] | sensor_all_bytes[5] << 8);
    x_gyr = (0x0000 | sensor_all_bytes[6] | sensor_all_bytes[7] << 8);
    y_gyr = (0x0000 | sensor_all_bytes[8] | sensor_all_bytes[9] << 8);
    z_gyr = (0x0000 | sensor_all_bytes[10] | sensor_all_bytes[11]<< 8);

    Serial.print("\rBMI270 data: Acc X, Acc Y, Acc Z ");
    Serial.print(x_acc);
    Serial.print(" , ");
    Serial.print(y_acc);
    Serial.print(" , ");
    Serial.print(z_acc);
    Serial.print(" , ");
    Serial.print(x_gyr);
    Serial.print(" , ");
    Serial.print(y_gyr);
    Serial.print(" , ");
    Serial.print(z_gyr);
    Serial.println();
  } else if (loopCounter == NUM_LOOPS_PER_SENSOR) {
    Serial.println("\nTesting Nicla Voice Magnetometer");
  } else { // loopCounter > NUM_LOOPS_PER_SENSOR
    bmm_sensor_address = 0x42;
    targetDevice = 1;
    s = NDP.transparentNiclaVoiceSensorRegRead(targetDevice, ndpCSPISpeedFactor, bmm_sensor_address, 8, sensor_all_bytes);
    if (debugTrace) {
      Serial.print("number of bytes outputted: ");
      Serial.println(s);

      for (int index = 0; index < 8; index++){
        Serial.println(sensor_all_bytes[index], HEX);
      }
    }

    x_mag = (0x0000 | sensor_all_bytes[0] >> 3 | sensor_all_bytes[1] << 5);
    y_mag = (0x0000 | sensor_all_bytes[2] >> 3 | sensor_all_bytes[3] << 5);
    z_mag = (0x0000 | sensor_all_bytes[4] >> 1 | sensor_all_bytes[5] << 7);
    hall  = (0x0000 | sensor_all_bytes[6] >> 2 | sensor_all_bytes[7] << 6);

    Serial.print("\rBMM150 data: Mag X, Mag Y, Mag Z, Hall ");
    Serial.print(x_mag);
    Serial.print(" , ");
    Serial.print(y_mag);
    Serial.print(" , ");
    Serial.print(z_mag);
    Serial.print(" , ");
    Serial.print(hall);
    Serial.println();
  }
  delay(100);

  // switch to other sensor again
  if (++loopCounter > 2 * NUM_LOOPS_PER_SENSOR) {
    loopCounter = 0;
  }
}
