class ImageDataProcessor {
    pixelFormatInfo = {
      "RGB565": {
        "convert": this.convertRGB565ToRGB888,
        "bytesPerPixel": 2
      },
      "GRAYSCALE": {
        "convert": this.convertGrayScaleToRGB888,
        "bytesPerPixel": 1
      },
      "RGB888": {
        "convert": this.convertToRGB888,
        "bytesPerPixel": 3
      },
      "BAYER": {
        "convert": null, // TODO
        "bytesPerPixel": 1
      }
    };

    constructor(context, mode = null, width = null, height = null) {
      this.context = context;
      this.canvas = context.canvas;
      
      if(mode) this.setMode(mode);
      if(width && height) this.setResolution(width, height);
    }
  
    setMode(mode) {
        this.mode = mode;
        this.bytesPerPixel = this.pixelFormatInfo[mode].bytesPerPixel;
    }

    setResolution(width, height) {
        this.width = width;
        this.height = height;
    }

    getTotalBytes() {
        return this.width * this.height * this.bytesPerPixel;
    }

    isConfigured() {
        return this.mode && this.width && this.height;
    }

    reset() {
        this.mode = null;
        this.bytesPerPixel = null;
        this.width = null;
        this.height = null;
    }
  
    convertRGB565ToRGB888(pixelValue) {
        // RGB565
        let r = (pixelValue >> (6 + 5)) & 0x1F;
        let g = (pixelValue >> 5) & 0x3F;
        let b = pixelValue & 0x1F;
        // RGB888 - amplify
        r <<= 3;
        g <<= 2;
        b <<= 3;
        return [r, g, b];
    }
    
    convertGrayScaleToRGB888(pixelValue) {
        return [pixelValue, pixelValue, pixelValue];
    }
    
    convertToRGB888(pixelValue){
       return [pixelValue[0], pixelValue[1], pixelValue[2]];
    }
  
    // Get the pixel value using big endian
    // Big-endian: the most significant byte comes first
    getPixelValue(sourceData, index) {        
        if (this.bytesPerPixel == 1) {
            return sourceData[index];
        } else if (this.bytesPerPixel == 2) {
            return (sourceData[index] << 8) | sourceData[index + 1];
        } else if (this.bytesPerPixel == 3) {
            return (sourceData[index] << 16) | (sourceData[index + 1] << 8) | sourceData[index + 2];
        } else if (this.bytesPerPixel == 4) {
            return (sourceData[index] << 24) | (sourceData[index + 1] << 16) | (sourceData[index + 2] << 8) | sourceData[index + 3];
        }
  
        return 0;
    }
  
    getImageData(bytes) {
        const BYTES_PER_ROW = this.width * this.bytesPerPixel;
      
        const imageData = this.context.createImageData(this.width, this.height);
        const dataContainer = imageData.data;
      
        for (let row = 0; row < this.height; row++) {
          for (let col = 0; col < this.width; col++) {
            const sourceDataIndex = (row * BYTES_PER_ROW) + (col * this.bytesPerPixel);
            const pixelValue = this.getPixelValue(bytes, sourceDataIndex, this.bytesPerPixel);
            const [r, g, b] = this.pixelFormatInfo[this.mode].convert(pixelValue);
      
            const pixelIndex = ((row * this.width) + col) * 4; // 4 channels: R, G, B, A
            dataContainer[pixelIndex] = r; // Red channel
            dataContainer[pixelIndex + 1] = g; // Green channel
            dataContainer[pixelIndex + 2] = b; // Blue channel
            dataContainer[pixelIndex + 3] = 255; // Alpha channel (opacity)
          }
        }
        return imageData;
      }
  }