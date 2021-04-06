#include "Arduino.h"
#include "pinDefinitions.h"

/* wiring_analog variables definition */
/* Flag to indicate whether the ADC config has been changed from the default one */
bool isAdcConfigChanged = false;

/* 
 * Configuration used for all the active ADC channels, it is initialized with the mbed default values
 * When it is changed, all the ADC channels are reconfigured accordingly 
 */
analogin_config_t adcCurrentConfig = {
    .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
    .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
    .gain       = NRF_SAADC_GAIN1_4,
    .reference  = NRF_SAADC_REFERENCE_VDD4,
    .acq_time   = NRF_SAADC_ACQTIME_10US,
    .mode       = NRF_SAADC_MODE_SINGLE_ENDED,
    .burst      = NRF_SAADC_BURST_DISABLED,
    .pin_p      = NRF_SAADC_INPUT_DISABLED,
    .pin_n      = NRF_SAADC_INPUT_DISABLED
};

void analogReference(uint8_t mode)
{
  nrf_saadc_reference_t reference = NRF_SAADC_REFERENCE_VDD4;
  nrf_saadc_gain_t gain = NRF_SAADC_GAIN1_4;
  if (mode == AR_VDD) {
    reference = NRF_SAADC_REFERENCE_VDD4;
    gain = NRF_SAADC_GAIN1_4;
  } else if (mode == AR_INTERNAL) {
    reference = NRF_SAADC_REFERENCE_INTERNAL;
    gain = NRF_SAADC_GAIN1;
  } else if (mode == AR_INTERNAL1V2) {
    reference = NRF_SAADC_REFERENCE_INTERNAL;
    gain = NRF_SAADC_GAIN1_2;
  } else if (mode == AR_INTERNAL2V4) {
    reference = NRF_SAADC_REFERENCE_INTERNAL;
    gain = NRF_SAADC_GAIN1_4;
  }
  adcCurrentConfig.reference = reference;
  adcCurrentConfig.gain = gain;
  analogUpdate();
}

void analogAcquisitionTime(uint8_t time)
{
  nrf_saadc_acqtime_t acqTime = NRF_SAADC_ACQTIME_10US;
  if (time == AT_3_US) {
    acqTime = NRF_SAADC_ACQTIME_3US;
  } else if (time == AT_5_US) {
    acqTime = NRF_SAADC_ACQTIME_5US;
  } else if (time == AT_10_US) {
    acqTime = NRF_SAADC_ACQTIME_10US;
  } else if (time == AT_15_US) {
    acqTime = NRF_SAADC_ACQTIME_15US;
  } else if (time == AT_20_US) {
    acqTime = NRF_SAADC_ACQTIME_20US;
  } else if (time == AT_40_US) {
    acqTime = NRF_SAADC_ACQTIME_40US;
  }
  adcCurrentConfig.acq_time = acqTime;
  analogUpdate();
}

AnalogPinDescription g_AAnalogPinDescription[] = {
    // A0 - A7
  { P0_5,  NULL },    // A0 - SENSOR_COMMON
  { P0_3,  NULL },    // A1 - SENSOR_CALIB
  { P0_4,  NULL },    // A2 - SENSOR_CAPTURE_A
  { P0_30, NULL },    // A3 - SENSOR_INPUT_ADC
  { P0_31, NULL },    // A4 - I2C_SDA_2
  { P0_2,  NULL },    // A5 - I2C_SCL_2
  { P0_28, NULL },    // A6 - V_REF
  { P0_29, NULL },    // A7 - VBAT_PROBE
};

PinDescription g_APinDescription[] = {
  // Triac
  { P0_13, NULL, NULL, NULL },     // 0 - CMD_TRIAC_1
  { P0_14, NULL, NULL, NULL },     // 1 - CMD_TRIAC_2
  { P0_15, NULL, NULL, NULL },     // 2 - CMD_TRIAC_3
  { P0_16, NULL, NULL, NULL },     // 3 - CMD_TRIAC_4

  // IRQ Channels
  { P1_15, NULL, NULL, NULL },     // 4 - IRQ_CH1
  { P1_14, NULL, NULL, NULL },     // 5 - IRQ_CH2
  { P0_26, NULL, NULL, NULL },     // 6 - IRQ_CH3
  { P0_6,  NULL, NULL, NULL },     // 7 - IRQ_CH4
  { P0_27, NULL, NULL, NULL },     // 8 - IRQ_CH5
  { P1_0,  NULL, NULL, NULL },     // 9 - IRQ_CH6

  // Sensors
  { P0_9,  NULL, NULL, NULL },     // 10 - SENSOR_CAPTURE

  // Sensors / Analogs
  { P0_5,  NULL, NULL, NULL },     // 11 - SENSOR_COMMON    - Analog
  { P0_3,  NULL, NULL, NULL },     // 12 - SENSOR_CALIB     - Analog
  { P0_4,  NULL, NULL, NULL },     // 13 - SENSOR_CAPTURE_A - Analog
  { P0_30, NULL, NULL, NULL },     // 14 - SENSOR_INPUT_ADC - Analog
  
  // I2C 2 / Analogs
  { P0_31, NULL, NULL, NULL },     // 15 - I2C_SDA1        - Analog
  { P0_2,  NULL, NULL, NULL },     // 16 - I2C_SCL1        - Analog

  // Power / Analogs 
  { P0_28, NULL, NULL, NULL },     // 17 - V_REF            - Analog
  { P0_29, NULL, NULL, NULL },     // 18 - VBAT_PROBE       - Analog

  // Pulse
  { P1_8,  NULL, NULL, NULL },     // 19 - PULSE_DIRECTION
  { P1_1,  NULL, NULL, NULL },     // 20 - PULSE_STROBE
  
  // MKR Connectors
  { P1_2,  NULL, NULL, NULL },     // 21 - ON_MKR1
  { P1_10, NULL, NULL, NULL },     // 22 - RXD_MKR1
  { P1_11, NULL, NULL, NULL },     // 23 - TXD_MKR1

  { P1_3,  NULL, NULL, NULL },     // 24 - ON_MKR2
  { P0_24, NULL, NULL, NULL },     // 25 - RXD_MKR2
  { P0_25, NULL, NULL, NULL },     // 26 - TXD_MKR2
  
  // I2C
  { P1_9,  NULL, NULL, NULL },     // 27 - I2C_SDA
  { P0_11, NULL, NULL, NULL },     // 28 - I2C_SCL
  // SD
  { P1_12, NULL, NULL, NULL },     // 29 - SD_CS

  // QSPI
  { P0_17, NULL, NULL, NULL },     // 30 - QSPIDCS
  { P0_19, NULL, NULL, NULL },     // 31 - GPIOCLK / SD_CLK_SCK
  { P0_20, NULL, NULL, NULL },     // 32 - QSPID0  / SD_CMD_MOSI
  { P0_21, NULL, NULL, NULL },     // 33 - QSPID1  / SD_DAT0_MISO
  { P0_22, NULL, NULL, NULL },     // 34 - QSPID2
  { P0_23, NULL, NULL, NULL },     // 35 - QSPID3

  // Power
  { P1_13, NULL, NULL, NULL },     // 36 - POWER_ON
  { P0_7,  NULL, NULL, NULL },     // 37 - GATED_19V_ENABLE
  { P0_10, NULL, NULL, NULL },     // 38 - GATED_VBAT_ENABLE
  { P0_12, NULL, NULL, NULL },     // 39 - GATED_3v3_ENABLE_N
};

extern "C" {
  unsigned int PINCOUNT_fn() {
    return (sizeof(g_APinDescription) / sizeof(g_APinDescription[0]));
  }
}

#include "nrf_rtc.h"

void initVariant() {
  // Errata Nano33BLE - I2C pullup is on SWO line, need to disable TRACE
  // was being enabled by nrfx_clock_anomaly_132
  // CoreDebug->DEMCR = 0;
  // NRF_CLOCK->TRACECONFIG = 0;  

  // FIXME: bootloader enables interrupt on COMPARE[0], which we don't handle
  // Disable it here to avoid getting stuck when OVERFLOW irq is triggered
  nrf_rtc_event_disable(NRF_RTC1, NRF_RTC_INT_COMPARE0_MASK);
  nrf_rtc_int_disable(NRF_RTC1, NRF_RTC_INT_COMPARE0_MASK);

  // Disable UARTE0 which is initially enabled by the bootloader
  nrf_uarte_task_trigger(NRF_UARTE0, NRF_UARTE_TASK_STOPRX); 
  while (!nrf_uarte_event_check(NRF_UARTE0, NRF_UARTE_EVENT_RXTO)) ; 
  NRF_UARTE0->ENABLE = 0; 
  NRF_UART0->ENABLE = 0; 

  // PWM Pins are anbled by defualt by MbedOS
  // Disable it to enable applications to use it.
  NRF_PWM_Type* PWM[] = {
    NRF_PWM0, NRF_PWM1, NRF_PWM2
#ifdef NRF_PWM3
    ,NRF_PWM3
#endif
  };

  for (int i = 0; i < (sizeof(PWM)/sizeof(PWM[0])); i++) {
    PWM[i]->ENABLE = 0;
    PWM[i]->PSEL.OUT[0] = 0xFFFFFFFFUL;
  }  
}

#ifdef SERIAL_CDC

static void utox8(uint32_t val, uint8_t* s) {
  for (int i = 0; i < 16; i=i+2) {
    int d = val & 0XF;
    val = (val >> 4);

    s[15 - i -1] = d > 9 ? 'A' + d - 10 : '0' + d;
    s[15 - i] = '\0';
  }
}

uint8_t getUniqueSerialNumber(uint8_t* name) {
  #define SERIAL_NUMBER_WORD_0  NRF_FICR->DEVICEADDR[1]
  #define SERIAL_NUMBER_WORD_1  NRF_FICR->DEVICEADDR[0]

  utox8(SERIAL_NUMBER_WORD_0, &name[0]);
  utox8(SERIAL_NUMBER_WORD_1, &name[16]);

  return 32;
}

void _ontouch1200bps_() {
  __disable_irq();
  NRF_POWER->GPREGRET = DFU_MAGIC_SERIAL_ONLY_RESET;
  NVIC_SystemReset();
}

#endif
