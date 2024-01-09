/* mbed Microcontroller Library
 * Copyright (c) 2006-2019 ARM Limited
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
#include "PTYSerial.h"

namespace arduino {

PTYSerial::PTYSerial(CMUXClass * parent) : _parent(parent) {
    _txbuf = new mbed::CircularBuffer<char, 1500U>();
    _rxbuf = new mbed::CircularBuffer<char, 1500U>();
}


PTYSerial::~PTYSerial()
{
}


int PTYSerial::close()
{
    // Does not let us pass a file descriptor. So how to close ?
    // Also, does it make sense to close a device type file descriptor?
    return 0;
}

off_t PTYSerial::seek(off_t offset, int whence)
{
    // lseek can be done theoratically, but is it sane to mark positions on
    // a dynamically growing/shrinking buffer system (from an interrupt
    // context)
    return -ESPIPE;
}

int PTYSerial::sync()
{
    api_lock();

    while (!_txbuf->empty()) {
        api_unlock();
        // Doing better than wait would require TxIRQ to also do wake() when
        // becoming empty. Worth it?
        thread_sleep_for(1);
        api_lock();
    }

    api_unlock();

    return 0;
}


void PTYSerial::sigio(mbed::Callback<void()> func)
{
    core_util_critical_section_enter();
    _sigio_cb = func;
    if (_sigio_cb) {
        short current_events = poll(0x7FFF);
        if (current_events) {
            _sigio_cb();
        }
    }
    core_util_critical_section_exit();
}

void PTYSerial::wake()
{
    if (_sigio_cb) {
        _sigio_cb();
    }
}


int PTYSerial::populate_rx_buffer(const char* buf, size_t sz) {
    size_t data_written = 0;

    const char *ptr = static_cast<const char *>(buf);

    if (sz == 0) {
        return 0;
    }

    api_lock();

    while (data_written < sz) {

        if (_rxbuf->full()) {
            if (!_blocking) {
                break;
            }
            do {
                api_unlock();
                // Should we have a proper wait?
                thread_sleep_for(1);
                api_lock();
            } while (_rxbuf->full());
        }

        while (data_written < sz && !_rxbuf->full()) {
            _rxbuf->push(*ptr++);
            data_written++;
        }
    }
    api_unlock();

    return data_written;
}

void PTYSerial::set_port(int id) {
 	_id = id;
}

int PTYSerial::get_port() {
 	return _id;
}
int PTYSerial::write(const void *buffer) {
    const char *buf_ptr = static_cast<const char *>(buffer);
    return write(buf_ptr, sizeof(buffer));
}

int PTYSerial::write(const void *buffer, size_t length) {
    const char *buf_ptr = static_cast<const char *>(buffer);
    int ret = _parent->populate_tx_buffer(buf_ptr, length, this->get_port());
    return ret;
}

int PTYSerial::write(const void *buffer, size_t length, int id) {
    const char *buf_ptr = static_cast<const char *>(buffer);
    int ret = _parent->populate_tx_buffer(buf_ptr, length, id);
    return ret;
}

int PTYSerial::available(){
    api_lock();
    int ret = _rxbuf->size();
    api_unlock();
    return ret;
}

ssize_t PTYSerial::read(void *buffer, size_t length)
{
    size_t data_read = 0;

    char *ptr = static_cast<char *>(buffer);

    if (length == 0) {
        return 0;
    }

    api_lock();
    while (data_read < length && !_rxbuf->empty()) {
        _rxbuf->pop(*ptr++);
        data_read++;
    }

    api_unlock();

    return data_read;
}

bool PTYSerial::is_full(void)
{
    return  _rxbuf->full();
}

void PTYSerial::api_lock(void)
{
    _mutex.lock();
}

void PTYSerial::api_unlock(void)
{
    _mutex.unlock();
}


short PTYSerial::poll(short events) const
{
    short revents = 0;
    // Check the Circular Buffer if space available for writing out
    if (!_rxbuf->empty()) {
        revents |= POLLIN;
    }

    // POLLHUP and POLLOUT are mutually exclusive
    if (!_txbuf->full()) {
        revents |= POLLOUT;
    }

    return revents;
}

int PTYSerial::enable_input(bool enabled)
{
    return 0;
}

int PTYSerial::enable_output(bool enabled)
{
    return 0;
}
} //namespace arduino