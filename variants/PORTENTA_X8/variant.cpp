#include "Arduino.h"
#include "pinDefinitions.h"

AnalogPinDescription g_AAnalogPinDescription[] = {
  { PF_11,       NULL },    // A0    ADC1_INP2
  { PA_6,        NULL },    // A1    ADC12_INP3
  { PF_13,       NULL },    // A2    ADC2_INP2
  { PB_1,        NULL },    // A3    ADC12_INP5
  { PC_4,        NULL },    // A4    ADC12_INP4
  { PF_7,        NULL },    // A5    ADC3_INP3
  { PF_9,        NULL },    // A6    ADC3_INP2
  { PF_5,        NULL }     // A7    ADC3_INP4
};

PinDescription g_APinDescription[] = {
  // D0 - D6
  { PF_8,        NULL, NULL, NULL },    // D0
  { PF_6,        NULL, NULL, NULL },    // D1
  { PF_3,        NULL, NULL, NULL },    // D2
  { PF_4,        NULL, NULL, NULL },    // D3
  { PF_12,       NULL, NULL, NULL },    // D4
  { PE_10,       NULL, NULL, NULL },    // D5
  { PE_11,       NULL, NULL, NULL },    // D6

  // D7 - D14
  { PF_11,       NULL },    // D7    A0
  { PA_6,        NULL },    // D8    A1
  { PF_13,       NULL },    // D9    A2
  { PB_1,        NULL },    // D10   A3
  { PC_4,        NULL },    // D11   A4
  { PF_7,        NULL },    // D12   A5
  { PF_9,        NULL },    // D13   A6
  { PF_5,        NULL },    // D14   A7

  // CAN
  { PD_1,        NULL, NULL, NULL }, // D15
  { PD_0,        NULL, NULL, NULL }, // D16
  { PB_6,        NULL, NULL, NULL }, // D17
  { PB_5,        NULL, NULL, NULL }, // D18

  // USART2
  { PD_3,        NULL, NULL, NULL }, // D19
  { PD_4,        NULL, NULL, NULL }, // D20
  { PD_5,        NULL, NULL, NULL }, // D21
  { PD_6,        NULL, NULL, NULL }, // D22

  // PWM
  { PC_7,        NULL, NULL, NULL }, // D23
  { PA_9,        NULL, NULL, NULL }, // D24
  { PA_10,       NULL, NULL, NULL }, // D25
  { PB_10,       NULL, NULL, NULL }, // D26
  { PA_11,       NULL, NULL, NULL }, // D27
  { PD_15,       NULL, NULL, NULL }, // D28
  { PA_8,        NULL, NULL, NULL }, // D29
  { PC_6,        NULL, NULL, NULL }, // D30
  { PA_12,       NULL, NULL, NULL }, // D31
  { PC_8,        NULL, NULL, NULL }, // D32

  // INTERNAL
  { PA_0,        NULL, NULL, NULL }, // D33
  { PC_1,        NULL, NULL, NULL }, // D34
  { PE_5,        NULL, NULL, NULL }, // D35

  // DUAL CONNECTION
  { PG_6,        NULL, NULL, NULL }, // D36
  { PG_7,        NULL, NULL, NULL }, // D37
  { PC_9,        NULL, NULL, NULL }, // D38
};

extern "C" {
  unsigned int PINCOUNT_fn() {
    return (sizeof(g_APinDescription) / sizeof(g_APinDescription[0]));
  }
}


void initVariant() {

}
