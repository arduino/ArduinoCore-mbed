/*
 * Copyright (c) 2020, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ALT1250_PPP_H_
#define ALT1250_PPP_H_

#ifdef TARGET_FF_ARDUINO
#ifndef MBED_CONF_ALT1250_PPP_TX
#define MBED_CONF_ALT1250_PPP_TX D1
#endif
#ifndef MBED_CONF_ALT1250_PPP_RX
#define MBED_CONF_ALT1250_PPP_RX D0
#endif
#endif /* TARGET_FF_ARDUINO */

#include "AT_CellularDevice.h"
#include "DigitalInOut.h"

namespace mbed {

class ALT1250_PPP : public AT_CellularDevice {
public:
    ALT1250_PPP(FileHandle *fh, PinName rst = NC, PinDirection pin_dir = PIN_OUTPUT, PinMode pin_mode = PullUp, bool value = 1);

protected: // AT_CellularDevice
    virtual AT_CellularContext *create_context_impl(ATHandler &at, const char *apn, bool cp_req = false, bool nonip_req = false);

    AT_CellularNetwork *open_network_impl(ATHandler &at);
    virtual nsapi_error_t soft_power_on();
    DigitalInOut _rst;
};

} // namespace mbed
#endif // ALT1250_PPP_H_
