#ifndef H747_System_h_
#define H747_System_h_

#include "Arduino.h"
#include "mbed.h"

enum class RTCBackup {
  DR0,      /* RESERVED Arduino Magic */
  DR1,      /* RESERVED OTA storage type */
  DR2,      /* RESERVED OTA offset */
  DR3,      /* RESERVED OTA update size */
  DR4,      /* RESERVED MCUboot scratch storage type */
  DR5,      /* RESERVED MCUboot scratch offset */
  DR6,      /* RESERVED MCUboot scratch size */
  DR7,      /* RESERVED MCUboot debug */
  DR8,      /* RESERVED Reset reason */
  DR9,
  DR10,
  DR11,
  DR12,
  DR13,
  DR14,
  DR15,
  DR16,
  DR17,
  DR18,
  DR19,
  DR20,
  DR21,
  DR22,
  DR23,
  DR24,
  DR25,
  DR26,
  DR27,
  DR28,
  DR29,
  DR30,
  DR31,
};

class STM32H747 {

public:
  virtual bool begin() = 0;
  virtual bool enterLowPower() = 0;
  static reset_reason_t  getResetReason();
  static uint32_t readBackupRegister(RTCBackup register);
  static void writeBackupRegister(RTCBackup register, uint32_t data);

protected:
  uint8_t readReg(uint8_t subAddress);
  void setRegister(uint8_t reg, uint8_t val);
  bool useInternalOscillator(bool lowspeed = false);
};

#endif