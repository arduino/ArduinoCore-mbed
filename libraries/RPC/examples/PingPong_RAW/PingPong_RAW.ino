#include "RPC.h"

void fatal_error() {
    while (true) {
        digitalWrite(LEDR, LOW);
        delay(500);
        digitalWrite(LEDR, HIGH);
        delay(500);
    }
}

void recv_callback(const uint8_t *buf, size_t len) {
    #ifdef CORE_CM7
    Serial.print("<= ");
    Serial.write(buf, len);
    Serial.println();
    #else
    const uint8_t msg[] = "Pong!";
    RPC.write(&msg[0], sizeof(msg), true);
    #endif
}

void setup() {
    #ifdef CORE_CM7
    Serial.begin(115200);
    while (!Serial) {

    }
    #endif

    if (!RPC.begin()) {
        fatal_error();
    }
    RPC.attach(recv_callback);

    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
}

void loop() {
    #ifdef CORE_CM7
    const uint8_t buf[] = "Ping!";
    Serial.print("=> ");
    Serial.write(buf, sizeof(buf));
    Serial.println();
    RPC.write(&buf[0], sizeof(buf));
    delay(100);
    #endif
}
