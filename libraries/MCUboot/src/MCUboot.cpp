#include "MCUboot.h"
#include "bootutil/bootutil_public.h"
#include "STM32H747_System.h"


void MCUboot::confirmSketch()
{
  boot_set_confirmed();
}

void MCUboot::applyUpdate(bool permanent)
{
  boot_set_pending(permanent == true ? 1 : 0);
}

void MCUboot::bootDebug(bool enable)
{
  unsigned int rtc_reg = STM32H747::readBackupRegister(RTCBackup::DR7);

  if(enable) {
    rtc_reg |= 0x00000001;
  } else {
    rtc_reg &= ~0x00000001;
  }

  return STM32H747::writeBackupRegister(RTCBackup::DR7, rtc_reg);
}
