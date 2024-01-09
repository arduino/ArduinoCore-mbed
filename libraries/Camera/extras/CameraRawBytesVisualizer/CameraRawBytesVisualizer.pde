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

// must match resolution used in the Arduino sketch
final int cameraWidth = 320;
final int cameraHeight = 240;

// Must match the image mode in the Arduino sketch
boolean useGrayScale = true;

// Must match the baud rate in the Arduino sketch
final int baudRate = 115200;

final int cameraBytesPerPixel = useGrayScale ? 1 : 2;
final int cameraPixelCount = cameraWidth * cameraHeight;
final int bytesPerFrame = cameraPixelCount * cameraBytesPerPixel;
final int timeout =  int((bytesPerFrame / float(baudRate / 10)) * 1000 * 2); // Twice the transfer rate

PImage myImage;
byte[] frameBuffer = new byte[bytesPerFrame];
int lastUpdate = 0;
boolean shouldRedraw = false;

void setup() {
  size(640, 480);  

  // If you have only ONE serial port active you may use this:
  //myPort = new Serial(this, Serial.list()[0], baudRate);          // if you have only ONE serial port active

  // If you know the serial port name
  //myPort = new Serial(this, "COM5", baudRate);                    // Windows
  //myPort = new Serial(this, "/dev/ttyACM0", baudRate);            // Linux
  myPort = new Serial(this, "/dev/cu.usbmodem14301", baudRate);     // Mac

  // wait for a full frame of bytes
  myPort.buffer(bytesPerFrame);  

  myImage = createImage(cameraWidth, cameraHeight, ALPHA);
  
  // Let the Arduino sketch know we're ready to receive data
  myPort.write(1);
}

void draw() {
  // Time out after a few seconds and ask for new data
  if(millis() - lastUpdate > timeout) {
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

int[] convertRGB565ToRGB888(short pixelValue){  
  //RGB565
  int r = (pixelValue >> (6+5)) & 0x01F;
  int g = (pixelValue >> 5) & 0x03F;
  int b = (pixelValue) & 0x01F;
  //RGB888 - amplify
  r <<= 3;
  g <<= 2;
  b <<= 3; 
  return new int[]{r,g,b};
}

void serialEvent(Serial myPort) {  
  lastUpdate = millis();
  
  // read the received bytes
  myPort.readBytes(frameBuffer);

  // Access raw bytes via byte buffer  
  ByteBuffer bb = ByteBuffer.wrap(frameBuffer);
  
  // Ensure proper endianness of the data for > 8 bit values.
  // The 1 byte bb.get() function will always return the bytes in the correct order.
  bb.order(ByteOrder.BIG_ENDIAN);

  int i = 0;

  while (bb.hasRemaining()) {
    if(useGrayScale){
      // read 8-bit pixel data
      byte pixelValue = bb.get();

      // set pixel color
      myImage.pixels[i++] = color(Byte.toUnsignedInt(pixelValue));
    } else {
      // read 16-bit pixel data
      int[] rgbValues = convertRGB565ToRGB888(bb.getShort());

      // set pixel RGB color
      myImage.pixels[i++] = color(rgbValues[0], rgbValues[1], rgbValues[2]);
    }       
  }
  
  myImage.updatePixels();
  
  // Ensures that the new image data is drawn in the next draw loop
  shouldRedraw = true;
  
  // Let the Arduino sketch know we received all pixels
  // and are ready for the next frame
  myPort.write(1);
}

void keyPressed() {
  if (key == ' ') {
    useGrayScale = !useGrayScale; // change boolean value of greyscale when space is pressed
  }
}