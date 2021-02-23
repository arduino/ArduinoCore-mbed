/*
  This sketch reads a raw Stream of RGB565 pixels
  from the Serial port and displays the frame on
  the window.
  Use with the Examples -> CameraCaptureRawBytes Arduino sketch.
  This example code is in the public domain.
*/

import processing.serial.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

Serial myPort;

// must match resolution used in the sketch
final int cameraWidth = 320;
final int cameraHeight = 240;
final int cameraBytesPerPixel = 1;
final int cameraPixelCount = cameraWidth * cameraHeight;
final int bytesPerFrame = cameraPixelCount * cameraBytesPerPixel;

PImage myImage;
byte[] frameBuffer = new byte[bytesPerFrame];
int lastUpdate = 0;
boolean shouldRedraw = false;

void setup() {
  size(640, 480);

  // if you have only ONE serial port active
  //myPort = new Serial(this, Serial.list()[0], 921600);          // if you have only ONE serial port active

  // if you know the serial port name
  //myPort = new Serial(this, "COM5", 921600);                    // Windows
  //myPort = new Serial(this, "/dev/ttyACM0", 921600);            // Linux
  myPort = new Serial(this, "/dev/cu.usbmodem14401", 921600);     // Mac

  // wait for full frame of bytes
  myPort.buffer(bytesPerFrame);  

  myImage = createImage(cameraWidth, cameraHeight, ALPHA);
  
  // Let the Arduino sketch know we're ready to receive data
  myPort.write(1);
}

void draw() {
  // Time out after 1.5 seconds and ask for new data
  if(millis() - lastUpdate > 1500) {
    println("Connection timed out.");
    myPort.clear();
    myPort.write(1);
  }
  
  if(shouldRedraw){    
    PImage img = myImage.copy();
    img.resize(640, 480);
    image(img, 0, 0);
    shouldRedraw = false;
  }
}

void serialEvent(Serial myPort) {
  lastUpdate = millis();
  
  // read the received bytes
  myPort.readBytes(frameBuffer);

  // Access raw bytes via byte buffer  
  ByteBuffer bb = ByteBuffer.wrap(frameBuffer);
  
  /* 
    Ensure proper endianness of the data for > 8 bit values.
    When using > 8bit values uncomment the following line and
    adjust the translation to the pixel color. 
  */     
  //bb.order(ByteOrder.BIG_ENDIAN);

  int i = 0;

  while (bb.hasRemaining()) {
    // read 8-bit pixel
    byte pixelValue = bb.get();

    // set pixel color
    myImage.pixels[i++] = color(Byte.toUnsignedInt(pixelValue));    
  }
  
  myImage.updatePixels();
  
  // Ensures that the new image data is drawn in the next draw loop
  shouldRedraw = true;
  
  // Let the Arduino sketch know we received all pixels
  // and are ready for the next frame
  myPort.write(1);
}
