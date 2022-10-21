#include "STM32H747_System.h"

void setup() {  
  Serial.begin(115200);
  while (!Serial) {}

  reset_reason_t resetReason = STM32H747::getResetReason();
  Serial.println(getString(resetReason));
}

String getString(reset_reason_t val) {
  switch (val){
  case RESET_REASON_POWER_ON:
    return "Reset Reason Power ON";
  case RESET_REASON_PIN_RESET:
    return "Reset Reason PIN Reset";
  case RESET_REASON_BROWN_OUT:
    return "Reset Reason Brown Out";
  case RESET_REASON_SOFTWARE:
    return "Reset Reason Software";
  case RESET_REASON_WATCHDOG:
    return "Reset Reason Watchdog";
  case RESET_REASON_LOCKUP:
    return "Reset Reason Lockup";
  case RESET_REASON_WAKE_LOW_POWER:
    return "Reset Reason Wake Low Power";
  case RESET_REASON_ACCESS_ERROR:
    return "Reset Reason Access Error";
  case RESET_REASON_BOOT_ERROR:
    return "Reset Reason Boot Error";
  case RESET_REASON_MULTIPLE:
    return "Reset Reason Multiple";
  case RESET_REASON_PLATFORM:
    return "Reset Reason Platform";
  case RESET_REASON_UNKNOWN:
    return "Reset Reason Unknown";
  default:
    return "N/A";
  }
}

void loop() {  
  delay(1000);
}
