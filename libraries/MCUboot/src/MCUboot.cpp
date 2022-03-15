#include "MCUboot.h"

extern "C" int boot_set_confirmed(void);

void MCUboot::confirmSketch()
{
  boot_set_confirmed();
}
