#include "anx7625_public.h"

struct anx7625 *anx7625;

void setup() {
  // put your setup code here, to run once:
  anx7625_i2c_probe(anx7625);
}

void loop() {
  // put your main code here, to run repeatedly:

}
