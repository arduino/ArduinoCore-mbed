
#ifdef ARDUINO_NICLA_VISION

#include "Nicla_Vision_System.h"
#include "Wire.h"
#include "mbed.h"

#define PMIC_ADDRESS 0x08

bool arduino::Nicla_Vision_System::begin()
{
  Wire1.begin();
}

void arduino::Nicla_Vision_System::enable3V3VDDIO_EXT() {
  //Power SW2 off
  STM32H747::setRegister(0x3B,0x80);
  delay(10);
  //Set SW2 to 3.3V
  STM32H747::setRegister(0x38,0x7);
  //Power SW2 on
  STM32H747::setRegister(0x3B,0x0F);
}

void arduino::Nicla_Vision_System::enable1V8VDDIO_EXT() {
  //Power SW2 off
  STM32H747::setRegister(0x3B,0x80);
  delay(10);
  //Set SW2 to 1.8V
  STM32H747::setRegister(0x38,0x4);
  //Power SW2 on
  STM32H747::setRegister(0x3B,0x0F);
}

void arduino::Nicla_Vision_System::disableVDDIO_EXT() {
  //Power SW2 off
  STM32H747::setRegister(0x3B,0x80);
}

bool arduino::Nicla_Vision_System::enterLowPower() {
  //USB Reset
  pinMode(PA_2, OUTPUT);
  digitalWrite(PA_2, LOW);

  //WiFi_Ctrl_On and BLE_Ctrl_On signals 
  pinMode(PF_14, OUTPUT);
  pinMode(PG_4, OUTPUT);
  digitalWrite(PF_14, LOW);
  digitalWrite(PG_4, LOW);
  
  //XSHUT TOF
  pinMode(PG_10, OUTPUT);
  digitalWrite(PG_10, LOW);
  
  //CRYPTO_EN
  pinMode(PG_0, OUTPUT);
  digitalWrite(PG_0, LOW);
  
  //FLASH CS
  pinMode(PG_6, OUTPUT);
  digitalWrite(PG_6, HIGH);

  //ACCELEROMETER IDLE
  pinMode(PF_6, OUTPUT);
  digitalWrite(PF_6, HIGH);

  //Stop non-lowpower Timer
  ArduinoTimer t = getTimer();
  t.stop();

  disableVDDIO_EXT();

  //LDOs off
  //Turn LDO1 off
  STM32H747::setRegister(0x4D,0x00);
  delay(10);
  //Turn LDO2 off
  STM32H747::setRegister(0x50,0x00);
  delay(10);
  //Turn LDO3 off
  STM32H747::setRegister(0x53,0x00);
  delay(10);

  //SW3 OFF
  STM32H747::setRegister(0x41,0x00);

  return STM32H747::useInternalOscillator();
}

arduino::Nicla_Vision_System Nicla_Vision;

#endif