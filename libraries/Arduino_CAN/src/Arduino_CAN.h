/*
 * Copyright (c) 2022 by Alexander Entinger <a.entinger@arduino.cc>
 * CAN library for Arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#ifndef ARDUINO_CORE_MBED_CAN_H_
#define ARDUINO_CORE_MBED_CAN_H_

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <Arduino.h>
#include <mbed.h>

#include "api/HardwareCAN.h"

/**************************************************************************************
 * COMPILE TIME CHECKS
 **************************************************************************************/

#if !(defined(ARDUINO_PORTENTA_H7_M7) || defined(ARDUINO_GIGA))
# error "CAN only available on Arduino Portenta H7 and Arduino Giga (of all ArduinoCore-mbed enabled boards)."
#endif

/**************************************************************************************
 * TYPEDEF
 **************************************************************************************/

typedef arduino::CanMsg CanMsg;

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

namespace arduino
{

/**************************************************************************************
 * CLASS DECLARATION
 **************************************************************************************/

class Arduino_CAN final : public HardwareCAN
{
public:
  Arduino_CAN(PinName const can_tx_pin, PinName const can_rx_pin);
  virtual ~Arduino_CAN() { }


  bool begin(CanBitRate const can_bitrate) override;
  void end() override;


  int write(CanMsg const & msg) override;
  size_t available() override;
  CanMsg read() override;

private:
  mbed::CAN _can;
  CanMsgRingbuffer _rx_msg_buf;
};

/**************************************************************************************
 * NAMESPACE
 **************************************************************************************/

} /* arduino */

/**************************************************************************************
 * EXTERN DECLARATION
 **************************************************************************************/

#if CAN_HOWMANY > 0
extern arduino::Arduino_CAN CAN;
#endif

#if CAN_HOWMANY > 1
extern arduino::Arduino_CAN CAN1;
#endif

#endif /* ARDUINO_CORE_MBED_CAN_H_ */
