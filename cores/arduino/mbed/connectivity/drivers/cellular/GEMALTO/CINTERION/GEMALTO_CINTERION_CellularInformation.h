/*
 * Copyright (c) 2018, Arm Limited and affiliates.
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

#ifndef GEMALTO_CINTERION_CELLULARINFORMATION_H_
#define GEMALTO_CINTERION_CELLULARINFORMATION_H_

#include "AT_CellularInformation.h"

namespace mbed {

class GEMALTO_CINTERION_CellularInformation: public AT_CellularInformation {
public:
    GEMALTO_CINTERION_CellularInformation(ATHandler &at, AT_CellularDevice &device);

public: // AT_CellularInformation
    virtual nsapi_error_t get_iccid(char *buf, size_t buf_size);
};

} /* namespace mbed */

#endif /* GEMALTO_CINTERION_CELLULARINFORMATION_H_ */
