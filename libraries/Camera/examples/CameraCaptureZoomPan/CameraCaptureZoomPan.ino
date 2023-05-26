/*
 * This example shows how to use the Nicla Vision to capture images from the camera
 * with a zoom window and send them over the serial port.
 * The zoom window will move from left to right and top to bottom 
 * in the predefined steps of pixels (ZOOM_X_STEP and ZOOM_Y_STEP).
 * 
 * Whenever the board sends a frame over the serial port, the blue LED will blink.
 * 
 * Instructions:
 * 1. Upload this sketch to Nicla Vision.
 * 2. Open the CameraRawBytesVisualizer.pde Processing sketch and change `useGrayScale` to `false`.
 * 3. Adjust the serial port in the Processing sketch to match the one used by Nicla Vision.
 * 4. Run the Processing sketch.
 * 
 * Initial author: Sebastian Romero @sebromero
 */

#include "camera.h"

#ifndef ARDUINO_NICLA_VISION
#error "This sketch only works on Nicla Vision."
#endif

#include "gc2145.h"
GC2145 galaxyCore;
Camera cam(galaxyCore);
#define IMAGE_MODE CAMERA_RGB565

#define CHUNK_SIZE 512  // Size of chunks in bytes
#define RESOLUTION CAMERA_R1600x1200 // Zoom in from the highest supported resolution
#define ZOOM_WINDOW_RESOLUTION CAMERA_R320x240

constexpr uint16_t ZOOM_WINDOW_WIDTH = 320;
constexpr uint16_t ZOOM_WINDOW_HEIGHT = 240;
constexpr uint16_t ZOOM_X_STEP = 100;
constexpr uint16_t ZOOM_Y_STEP = 100;

FrameBuffer frameBuffer;
uint32_t currentZoomX = 0;
uint32_t currentZoomY = 0;
uint32_t maxZoomX = 0; // Will be calculated in setup()
uint32_t maxZoomY = 0; // Will be calculated in setup()


void blinkLED(uint32_t count = 0xFFFFFFFF)
{
  pinMode(LED_BUILTIN, OUTPUT);  
  
  while (count--) {
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED on (HIGH is the voltage level)
    delay(50);                       // wait for a second
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED off by making the voltage LOW
    delay(50);                       // wait for a second
  }
}

void setup() {
  // Init the cam QVGA, 30FPS
  if (!cam.begin(RESOLUTION, IMAGE_MODE, 30)) {
    blinkLED();
  }

  blinkLED(5);
  
  pinMode(LEDB, OUTPUT);
  digitalWrite(LEDB, HIGH);

  // Flips the image vertically
  cam.setVerticalFlip(true);

  // Mirrors the image horizontally
  cam.setHorizontalMirror(true);

  // Calculate the max zoom window position
  maxZoomX = cam.getResolutionWidth() - ZOOM_WINDOW_WIDTH;
  maxZoomY = cam.getResolutionHeight() - ZOOM_WINDOW_HEIGHT;

  // Set the zoom window to 0,0  
  cam.zoomTo(ZOOM_WINDOW_RESOLUTION, currentZoomX, currentZoomY);
}

void sendFrame(){
  // Grab frame and write to serial
  if (cam.grabFrame(frameBuffer, 3000) == 0) {    
    byte* buffer = frameBuffer.getBuffer();
    size_t bufferSize = cam.frameSize();
    digitalWrite(LEDB, LOW);
    
    // Split buffer into chunks
    for(size_t i = 0; i < bufferSize; i += CHUNK_SIZE) {
      size_t chunkSize = min(bufferSize - i, CHUNK_SIZE);
      Serial.write(buffer + i, chunkSize);
      Serial.flush();
      delay(1);  // Small delay to allow the receiver to process the data
    }    
    
    digitalWrite(LEDB, HIGH);
  } else {
    blinkLED(20);
  }
}

void loop() {
  if(!Serial) {    
    Serial.begin(115200);
    while(!Serial);    
  }

  if(!Serial.available()) return;
  byte request = Serial.read();

  if(request == 1){
    sendFrame();
    currentZoomX += ZOOM_X_STEP;

    if(currentZoomX > maxZoomX){
      currentZoomX = 0;
      currentZoomY += ZOOM_Y_STEP;
      if(currentZoomY > maxZoomY){
        currentZoomY = 0;
      }
    }
    cam.zoomTo(ZOOM_WINDOW_RESOLUTION, currentZoomX, currentZoomY); 
  }

}
