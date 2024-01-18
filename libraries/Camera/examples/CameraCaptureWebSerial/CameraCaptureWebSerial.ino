/*
 * This example shows how to capture images from the camera and send them over Web Serial.
 * 
 * There is a companion web app that receives the images and displays them in a canvas.
 * It can be found in the "extras" folder of this library.
 * The on-board LED lights up while the image is being sent over serial.
 * 
 * Instructions:
 * 1. Make sure the correct camera is selected in the #include section below by uncommenting the correct line.
 * 2. Upload this sketch to your camera-equipped board.
 * 3. Open the web app in a browser (Chrome or Edge) by opening the index.html file 
 * in the "WebSerialCamera" folder which is located in the "extras" folder.
 * 
 * Initial author: Sebastian Romero @sebromero
 */

#include "camera.h"

#ifdef ARDUINO_NICLA_VISION
  #include "gc2145.h"
  GC2145 galaxyCore;
  Camera cam(galaxyCore);
  #define IMAGE_MODE CAMERA_RGB565
#elif defined(ARDUINO_PORTENTA_H7_M7)
  // uncomment the correct camera in use
  #include "hm0360.h"
  HM0360 himax;
  // #include "himax.h";
  // HM01B0 himax;
  Camera cam(himax);
  #define IMAGE_MODE CAMERA_GRAYSCALE
#elif defined(ARDUINO_GIGA)
  #include "ov767x.h"
  // uncomment the correct camera in use
  OV7670 ov767x;
  // OV7675 ov767x;
  Camera cam(ov767x);
  #define IMAGE_MODE CAMERA_RGB565
#else
#error "This board is unsupported."
#endif

/*
Other buffer instantiation options:
  FrameBuffer fb(0x30000000);
  FrameBuffer fb(320,240,2);

If resolution higher than 320x240 is required, please use external RAM via
  #include "SDRAM.h"
  FrameBuffer fb(SDRAM_START_ADDRESS);
  ...
  // and adding in setup()
  SDRAM.begin();
*/
constexpr uint16_t CHUNK_SIZE = 512;  // Size of chunks in bytes
constexpr uint8_t RESOLUTION  = CAMERA_R320x240; // CAMERA_R160x120
constexpr uint8_t CONFIG_SEND_REQUEST = 2;
constexpr uint8_t IMAGE_SEND_REQUEST = 1;

uint8_t START_SEQUENCE[4] = { 0xfa, 0xce, 0xfe, 0xed };
uint8_t STOP_SEQUENCE[4] = { 0xda, 0xbb, 0xad, 0x00 };
FrameBuffer fb;

/**
 * Blinks the LED a specified number of times.
 * @param ledPin The pin number of the LED.
 * @param count The number of times to blink the LED. Default is 0xFFFFFFFF.
 */
void blinkLED(int ledPin, uint32_t count = 0xFFFFFFFF) { 
  while (count--) {
    digitalWrite(ledPin, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(50);                       // wait for a second
    digitalWrite(ledPin, HIGH); // turn the LED off by making the voltage LOW
    delay(50);                       // wait for a second
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  
  pinMode(LEDR, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LEDR, HIGH);

  // Init the cam QVGA, 30FPS
  if (!cam.begin(RESOLUTION, IMAGE_MODE, 30)) {
    blinkLED(LEDR);
  }

  blinkLED(LED_BUILTIN, 5);
}

/**
 * Sends a chunk of data over a serial connection.
 * 
 * @param buffer The buffer containing the data to be sent.
 * @param bufferSize The size of the buffer.
 */
void sendChunk(uint8_t* buffer, size_t bufferSize){
  Serial.write(buffer, bufferSize);
  Serial.flush();
  delay(1); // Optional: Add a small delay to allow the receiver to process the chunk
}

/**
 * Sends a frame of camera image data over a serial connection.
 */
void sendFrame(){
  // Grab frame and write to serial
  if (cam.grabFrame(fb, 3000) == 0) {    
    byte* buffer = fb.getBuffer();
    size_t bufferSize = cam.frameSize();
    digitalWrite(LED_BUILTIN, LOW);
    
    sendChunk(START_SEQUENCE, sizeof(START_SEQUENCE));

    // Split buffer into chunks
    for(size_t i = 0; i < bufferSize; i += CHUNK_SIZE) {
      size_t chunkSize = min(bufferSize - i, CHUNK_SIZE);
      sendChunk(buffer + i, chunkSize);
    }    
    
    sendChunk(STOP_SEQUENCE, sizeof(STOP_SEQUENCE));
    
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    blinkLED(20);
  }
}

/**
 * Sends the camera configuration over a serial connection.
 * This is used to configure the web app to display the image correctly.
 */
void sendCameraConfig(){
  Serial.write(IMAGE_MODE);
  Serial.write(RESOLUTION);
  Serial.flush();
  delay(1);
}

void loop() {
  if(!Serial) {    
    Serial.begin(115200);
    while(!Serial);    
  }

  if(!Serial.available()) return;

  byte request = Serial.read();

  switch(request){
    case IMAGE_SEND_REQUEST:
      sendFrame();
      break; 
    case CONFIG_SEND_REQUEST:
      sendCameraConfig();
      break;
  }
  
}
