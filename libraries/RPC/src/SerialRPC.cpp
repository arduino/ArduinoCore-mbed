// Copyright (c) 2024 Arduino SA
// SPDX-License-Identifier: MPL-2.0
#include "SerialRPC.h"
#include "RPC.h"

size_t arduino::SerialRPCClass::write(uint8_t* buf, size_t len) {
    tx_buffer.clear();
    for (size_t i=0; i < len; i++) {
        tx_buffer.push_back(buf[i]);
    }
    RPC.send("tty", tx_buffer);
    return len;
}

int arduino::SerialRPCClass::begin(long unsigned int, uint16_t) {
    if (RPC.begin() == 0) {
        return 0;
    }
    RPC.bind("tty", mbed::callback(this, &SerialRPCClass::onWrite));
    return 1;
}

arduino::SerialRPCClass::operator bool() {
    return RPC;
}

arduino::SerialRPCClass SerialRPC;
