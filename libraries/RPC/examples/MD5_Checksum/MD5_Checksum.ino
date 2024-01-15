#include "RPC.h"
#include "MD5.h"

size_t hash_in_count = 0;
size_t hash_out_count = 0;

#ifdef CORE_CM4
size_t data_buf_size = 0;
#else
size_t data_buf_size = 256;
#endif

typedef std::vector<byte> vec_t;

void fatal_error() {
    while (true) {
        digitalWrite(LEDR, LOW);
        delay(500);
        digitalWrite(LEDR, HIGH);
        delay(500);
    }
}

vec_t hash_block(vec_t &buf) {
    MD5_CTX context;
    MD5::MD5Init(&context);
    MD5::MD5Update(&context, buf.data(), buf.size());

    vec_t hash(16);
    MD5::MD5Final(&hash[0], &context);
    return hash;
}

vec_t md5hash(vec_t &buf) {
    hash_out_count++;
    return hash_block(buf);
}

#ifdef CORE_CM4
// Called by the host to set the data buffer size.
size_t set_buffer_size(size_t size) {
    data_buf_size = size;
    return 0;
}
#endif

void setup() {
    #ifdef CORE_CM7
    Serial.begin(115200);
    while (!Serial) {
      
    }
    #endif
    
    if (!RPC.begin()) {
      fatal_error();
    }
    RPC.bind("md5hash", md5hash);    
    #ifdef CORE_CM4
    RPC.bind("set_buffer_size", set_buffer_size);    
    #else
    // Introduce a brief delay to allow the M4 sufficient time
    // to bind remote functions before invoking them.
    delay(100);
    auto ret = RPC.call("set_buffer_size", data_buf_size).as<size_t>();
    #endif
        
    pinMode(LEDR, OUTPUT);
    pinMode(LEDG, OUTPUT);
}

void loop() {
    static vec_t data;
    static uint32_t ticks_start = millis();
        
    // Wait for the host processor to set the data buffer size.
    if (data_buf_size == 0) {
        return;
    } else if (data.size() == 0) {
        data.resize(data_buf_size, 0);
    }

    // Fill the buffer with random data.
    for (int i=0; i<data.size(); i++) {
        data[i] = random(256);
    }

    // Send buffer to other core and read back the checksum.
    auto ret = RPC.call("md5hash", data).as<vec_t>();

    // Calculate checksum and compare with the received checksum.
    vec_t hash = hash_block(data);
    if (memcmp(&hash[0], &ret[0], 16) != 0) {
        fatal_error();
    }
    
    #ifdef CORE_CM4
    if ((hash_in_count % 512) == 0) {
        digitalWrite(LEDG, LOW);
        delay(10);
        digitalWrite(LEDG, HIGH);
        delay(10);
    }
    #endif
 
    #ifdef CORE_CM7
    if ((hash_in_count % 16) == 0) {
      float khs = (hash_in_count + hash_out_count) / (float) (millis() - ticks_start);
      Serial.println("Generated: " + String(hash_out_count) + " Received: " + String(hash_in_count) + " " + String(khs) +" KH/S");
    }
    //delay(1);
    #endif
    
    hash_in_count++;
}
