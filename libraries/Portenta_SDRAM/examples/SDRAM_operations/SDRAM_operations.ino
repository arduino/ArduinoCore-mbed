/*
    How to interact with external SDRAM on Portenta H7

    The board comes with an hefty 8MB of external fast RAM, which can be used:
    - as a framebuffer (raw mode)
    - as an expansion of on-chip RAM to store "standard" data

    This example shows both the usages
*/

#include "SDRAM.h"

REDIRECT_STDOUT_TO(Serial);

void nonFrameBuffer() {
    // Initilize SDRAM for non-framebuffer operations
    SDRAM.begin(); // is the same as SDRAM.begin(SDRAM_START_ADDRESS);

    // Now we can malloc() and free() in the whole RAM space
    // For example, let's create a 7MB array
    uint8_t* myVeryBigArray = (uint8_t*)SDRAM.malloc(7 * 1024 * 1024);

    // and a small one
    uint8_t* mySmallArray = (uint8_t*)SDRAM.malloc(128);

    // and use then as usual
    for (int i = 0; i<128; i++) {
        myVeryBigArray[i] = i;
        mySmallArray[i] = i*2;
    }

    // free the memory when you don't need them anymore
    SDRAM.free(myVeryBigArray);
}

void frameBuffer() {
    // In case we want a framebuffer-like area at the beginning of the flash,
    // simply initialize the memory as

    SDRAM.begin(SDRAM_START_ADDRESS + 2 * 1024 * 1024);
    // 2MB of contiguous memory available at the beginning

    uint32_t* framebuffer = (uint32_t*)SDRAM_START_ADDRESS;

    // We can't allocate anymore the huge 7MB array

    uint8_t* myVeryBigArray = (uint8_t*)SDRAM.malloc(7 * 1024 * 1024);
    if (myVeryBigArray == NULL) {
        Serial.println("Oops, too big :)");
    }

}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    frameBuffer();
    // Uncomment to test the other functionality 
    // nonFrameBuffer();

    // Sort of memtest for stability, useful for testing when overclocking
    if (SDRAM.test()) {
        Serial.println("SDRAM completely functional");
    }
}

void loop() {
    
}