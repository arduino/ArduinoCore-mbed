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
 * 3. Open the web app in a browser (Chrome or Edge) by opening the index.html file in the "extras" folder.
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
#define CHUNK_SIZE 512  // Size of chunks in bytes
#define RESOLUTION  CAMERA_R320x240 // CAMERA_R160x120
constexpr uint8_t START_SEQUENCE[4] = { 0xfa, 0xce, 0xfe, 0xed };
constexpr uint8_t STOP_SEQUENCE[4] = { 0xda, 0xbb, 0xad, 0x00 };
FrameBuffer fb;

unsigned long lastUpdate = 0;

void blinkLED(uint32_t count = 0xFFFFFFFF) { 
  while (count--) {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(50);                       // wait for a second
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED off by making the voltage LOW
    delay(50);                       // wait for a second
  }
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  

  // Init the cam QVGA, 30FPS
  if (!cam.begin(RESOLUTION, IMAGE_MODE, 30)) {
    blinkLED();
  }

  blinkLED(5);
}

void sendFrame(){
  // Grab frame and write to serial
  if (cam.grabFrame(fb, 3000) == 0) {    
    byte* buffer = fb.getBuffer();
    size_t bufferSize = cam.frameSize();
    digitalWrite(LED_BUILTIN, LOW);
    
    Serial.write(START_SEQUENCE, sizeof(START_SEQUENCE));
    Serial.flush();
    delay(1);

    // Split buffer into chunks
    for(size_t i = 0; i < bufferSize; i += CHUNK_SIZE) {
      size_t chunkSize = min(bufferSize - i, CHUNK_SIZE);
      Serial.write(buffer + i, chunkSize);
      Serial.flush();
      delay(1);  // Optional: Add a small delay to allow the receiver to process the chunk
    }    
    Serial.write(STOP_SEQUENCE, sizeof(STOP_SEQUENCE));
    Serial.flush();
    delay(1);
    
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    blinkLED(20);
  }
}

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
    case 1:
      sendFrame();
      break; 
    case 2:
      sendCameraConfig();
      break;
  }
  
}
