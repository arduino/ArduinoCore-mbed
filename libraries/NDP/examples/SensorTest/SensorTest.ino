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

int loopCounter = 0;

#define CHECK_STATUS(s) do { if (s) {Serial.print("SPI access error in line "); Serial.println(__LINE__); for(;;);}} while (0)

void setup() {
  int s;
  uint8_t __attribute__((aligned(4))) sensor_data[16];
  int retry_sensor_init = 0;

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

  do {
    if (retry_sensor_init) {
      Serial.print("Reinit attempt ");
      Serial.println(retry_sensor_init);
    }
    // 1st read will place the sensor in SPI mode, 2nd read is real read
    s = NDP.sensorBMI270Read(0x0, 1, sensor_data);
    CHECK_STATUS(s);

    s = NDP.sensorBMI270Read(0x0, 1, sensor_data);
    CHECK_STATUS(s);
    Serial.print("BMI270 chip ID is (expected is 0x24): ");
    Serial.println(sensor_data[0], HEX);

    // soft reset
    s = NDP.sensorBMI270Write(0x7e, 0xb6);
    CHECK_STATUS(s);
    delay(20);

    // back to SPI mode after software reset
    s = NDP.sensorBMI270Read(0x0, 1, sensor_data);
    CHECK_STATUS(s);
    s = NDP.sensorBMI270Read(0x0, 1, sensor_data);
    CHECK_STATUS(s);

    s = NDP.sensorBMI270Read(0x21, 1, sensor_data);
    CHECK_STATUS(s);
    Serial.print("[After reset] BMI270 Status Register at address 0x21 is (expected is 0x00): 0x");
    Serial.println(sensor_data[0], HEX);

    // disable PWR_CONF.adv_power_save
    s = NDP.sensorBMI270Write(0x7c, 0x00);
    CHECK_STATUS(s);
    delay(20);

    // prepare config load INIT_CTRL = 0x00
    s = NDP.sensorBMI270Write(0x59, 0x00);
    CHECK_STATUS(s);
    delay(200);

    // burst write to INIT_DATA
    Serial.print("BMI270 init starting...");
    s = NDP.sensorBMI270Write(0x5e,
              sizeof(bmi270_maximum_fifo_config_file),
              (uint8_t*)bmi270_maximum_fifo_config_file);
    CHECK_STATUS(s);
    Serial.println("... done!");

    s = NDP.sensorBMI270Write(0x59, 0x01);
    CHECK_STATUS(s);
    delay(200);

    s = NDP.sensorBMI270Read(0x21, 1, sensor_data);
    CHECK_STATUS(s);
    Serial.print("BMI270 Status Register at address 0x21 is (expected is 0x01): 0x");
    Serial.println(sensor_data[0], HEX);
    if (sensor_data[0] != 1) {
      retry_sensor_init++;
    } else {
      retry_sensor_init = 0;
    }
  } while (retry_sensor_init);

  // configuring device to normal power mode with both Accelerometer and gyroscope working
  s = NDP.sensorBMI270Write(0x7d, 0x0e);
  CHECK_STATUS(s);
  s = NDP.sensorBMI270Write(0x40, 0xa8);
  CHECK_STATUS(s);
  s = NDP.sensorBMI270Write(0x41, 0x00); // +/- 2g
  CHECK_STATUS(s);
  s = NDP.sensorBMI270Write(0x42, 0xa9); // odr 200, OSR2, noise ulp, filter hp
  CHECK_STATUS(s);
  s = NDP.sensorBMI270Write(0x43, 0x11); // gyr range_1000, ois range_2000
  CHECK_STATUS(s);
  s = NDP.sensorBMI270Write(0x7c, 0x02);

  s = NDP.sensorBMM150Write(0x4b, 0x01);
  CHECK_STATUS(s);
  delay(20);
  s = NDP.sensorBMM150Read(0x4b, 1, sensor_data);
  CHECK_STATUS(s);
  Serial.print("BMM150 power control byte at address 0x4B is (expected is 0x01): 0x");
  Serial.println(sensor_data[0], HEX);

  s = NDP.sensorBMM150Write(0x4c, 0x00);
  CHECK_STATUS(s);

  s = NDP.sensorBMM150Read(0x40, 1, sensor_data);
  CHECK_STATUS(s);
  Serial.print("BMM150 chip ID at address 0x40 is (expected is 0x32): 0x");
  Serial.println(sensor_data[0], HEX);
}

#define NUM_LOOPS_PER_SENSOR 15
void loop() {
  uint8_t __attribute__((aligned(4)))  sensor_data[16];

  int16_t x_acc, y_acc, z_acc, x_gyr, y_gyr, z_gyr  ;
  int16_t x_mag, y_mag, z_mag, hall;
  int s;

  if (loopCounter < NUM_LOOPS_PER_SENSOR) {
    if (!loopCounter) {
      Serial.println("\nTesting Nicla Voice BMI270 Sensor");
    }

    s = NDP.sensorBMI270Read(0xc, 16, &sensor_data[0]);
    CHECK_STATUS(s);
    x_acc = (0x0000 | sensor_data[0] | sensor_data[1] << 8);
    y_acc = (0x0000 | sensor_data[2] | sensor_data[3] << 8);
    z_acc = (0x0000 | sensor_data[4] | sensor_data[5] << 8);
    x_gyr = (0x0000 | sensor_data[6] | sensor_data[7] << 8);
    y_gyr = (0x0000 | sensor_data[8] | sensor_data[9] << 8);
    z_gyr = (0x0000 | sensor_data[10] | sensor_data[11]<< 8);

    Serial.print("\rBMI270 data: Acc X/Y/Z   Gyr X/Y/Z ");
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
    Serial.println("\nTesting Nicla Voice BMM150 Sensor");
  } else { // loopCounter > NUM_LOOPS_PER_SENSOR
    s = NDP.sensorBMM150Read(0x42, 8, sensor_data);
    CHECK_STATUS(s);
    x_mag = (0x0000 | sensor_data[0] >> 3 | sensor_data[1] << 5);
    y_mag = (0x0000 | sensor_data[2] >> 3 | sensor_data[3] << 5);
    z_mag = (0x0000 | sensor_data[4] >> 1 | sensor_data[5] << 7);
    hall  = (0x0000 | sensor_data[6] >> 2 | sensor_data[7] << 6);

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

  if (++loopCounter > 2 * NUM_LOOPS_PER_SENSOR) {
    loopCounter = 0;
  }
}
