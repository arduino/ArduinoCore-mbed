#include "anx7625_public.h"
#include "RPC_internal.h"

struct anx7625 *anx7625;

void setup() {
  RPC1.begin();
  delay(2000);
  // put your setup code here, to run once:
  anx7625_i2c_probe(anx7625);
  anx7625_bridge_enable(anx7625);
}

void loop() {
  // put your main code here, to run repeatedly:

}
