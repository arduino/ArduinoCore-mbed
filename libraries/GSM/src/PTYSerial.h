/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
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

#ifndef PTYSERIAL_H
#define PTYSERIAL_H

#include "CMUXClass.h"
#include "platform/platform.h"

// #include "platform/mbed_poll.h"
// #include "platform/mbed_thread.h"
#include "CellularLog.h"
#include "CMUXClass.h"


#include "platform/FileHandle.h"
#include "drivers/InterruptIn.h"
#include "platform/PlatformMutex.h"
#include "platform/CircularBuffer.h"
#include "platform/NonCopyable.h"

#ifndef MBED_CONF_DRIVERS_UART_SERIAL_RXBUF_SIZE
#define MBED_CONF_DRIVERS_UART_SERIAL_RXBUF_SIZE  256
#endif

#ifndef MBED_CONF_DRIVERS_UART_SERIAL_TXBUF_SIZE
#define MBED_CONF_DRIVERS_UART_SERIAL_TXBUF_SIZE  256
#endif

namespace arduino {
/**
 * \defgroup drivers_BufferedSerial BufferedSerial class
 * \ingroup drivers-public-api-uart
 * @{
 */

/** Class providing buffered UART communication functionality using separate
 *  circular buffer for send and receive channels
 *
 */

class PTYSerial : public mbed::FileHandle
{

public:
    PTYSerial(CMUXClass * parent);
    ~PTYSerial() override;
    int populate_rx_buffer(const char* buf, size_t sz);
    int write(const void *buffer, size_t length) override;
    int write(const void *buffer, size_t length, int id);
    int write(const void *buffer);
    using mbed::FileHandle::readable;
    using mbed::FileHandle::writable;
    void set_port(int id);
    int get_port();

    ssize_t read(void *buffer, size_t length) override;
    int close() override;
    off_t seek(off_t offset, int whence) override;
    int sync() override;
    short poll(short events) const final;
    int enable_input(bool enabled) override;
    int enable_output(bool enabled) override;
    int available();
    bool is_full();
    void sigio(mbed::Callback<void()> func) override;
    int set_blocking(bool blocking) override
    {
        _blocking = blocking;
        return 0;
    }

    bool is_blocking() const override
    {
        return _blocking;
    }
    int _id = -1;
private:
    CMUXClass * _parent;
    mbed::CircularBuffer<char, 1500U> * _txbuf;
    mbed::CircularBuffer<char, 1500U> * _rxbuf;

    mbed::Callback<void()> _sigio_cb;


private:
    void api_lock(void);
    void api_unlock(void);

    PlatformMutex _mutex;
    bool _blocking = true;
    void wake(void);

};

/** @}*/

} //namespace arduino


#endif //MBED_PTYSERIAL_H
