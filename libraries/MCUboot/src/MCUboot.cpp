#include "MCUboot.h"
#include "bootutil.h"

void MCUboot::confirmSketch()
{
  boot_set_confirmed();
}

void MCUboot::applyUpdate(int permanent)
{
  boot_set_pending(permanent);
}

void MCUboot::bootDebug(int enable)
{
  boot_set_debug(enable);
}
