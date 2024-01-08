/**
 * Represents an image data processor that converts raw image data to a specified pixel format.
 * 
 * @author Sebastian Romero
 */
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
        "convert": () => {throw new Error("BAYER conversion not implemented.")},
        "bytesPerPixel": 1
      }
    };

    /**
     * Creates a new instance of the imageDataProcessor class.
     * @param {string|null} mode - The image mode of the image data processor. (Optional)
     * Possible values: RGB565, GRAYSCALE, RGB888, BAYER
     * @param {number|null} width - The width of the image data processor. (Optional)
     * @param {number|null} height - The height of the image data processor. (Optional)
     */
    constructor(mode = null, width = null, height = null) {
      if(mode) this.setImageMode(mode);
      if(width && height) this.setResolution(width, height);
    }
  
    /**
     * Sets the image mode of the image data processor.
     * Possible values: RGB565, GRAYSCALE, RGB888, BAYER
     * 
     * @param {string} mode - The image mode of the image data processor.
     */
    setImageMode(mode) {
        this.mode = mode;
        this.bytesPerPixel = this.pixelFormatInfo[mode].bytesPerPixel;
    }

    /**
     * Sets the resolution of the target image.
     * @param {number} width - The width of the resolution.
     * @param {number} height - The height of the resolution.
     */
    setResolution(width, height) {
        this.width = width;
        this.height = height;
    }

    /**
     * Calculates the total number of bytes in the image data 
     * based on the current image mode and resolution.
     * 
     * @returns {number} The total number of bytes.
     */
    getTotalBytes() {
        return this.width * this.height * this.bytesPerPixel;
    }

    /**
     * Resets the state of the imageDataProcessor.
     * This resets the image mode, resolution, and bytes per pixel.
     */
    reset() {
        this.mode = null;
        this.bytesPerPixel = null;
        this.width = null;
        this.height = null;
    }
  
    /**
     * Converts a pixel value from RGB565 format to RGB888 format.
     * @param {number} pixelValue - The pixel value in RGB565 format.
     * @returns {number[]} - The RGB888 pixel value as an array of three values [R, G, B].
     */
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
    
    /**
     * Converts a grayscale pixel value to RGB888 format.
     * @param {number} pixelValue - The grayscale pixel value.
     * @returns {number[]} - The RGB888 pixel value as an array of three values [R, G, B].
     */
    convertGrayScaleToRGB888(pixelValue) {
        return [pixelValue, pixelValue, pixelValue];
    }
    
    /**
     * Converts a pixel value to RGB888 format.
     * @param {number} pixelValue - The pixel value to convert.
     * @returns {number[]} - The RGB888 pixel value as an array of three values [R, G, B].
     */
    convertToRGB888(pixelValue){
       return pixelValue;
    }
    
    /**
     * Retrieves the pixel value from the source data at the specified index
     * using big endian: the most significant byte comes first.
     * 
     * @param {Uint8Array} sourceData - The source data array.
     * @param {number} index - The index of the pixel value in the source data array.
     * @returns {number} The pixel value.
     */
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
  
    /**
     * Retrieves the image data from the given bytes by converting each pixel value.
     * 
     * @param {Uint8Array} bytes - The raw byte array containing the image data.
     * @returns {Uint8ClampedArray} The image data as a Uint8ClampedArray containing RGBA values.
     */
    convertToPixelData(bytes) {
        const BYTES_PER_ROW = this.width * this.bytesPerPixel;
        const dataContainer = new Uint8ClampedArray(this.width * this.height * 4); // 4 channels: R, G, B, A
      
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
        return dataContainer;
      }
  }