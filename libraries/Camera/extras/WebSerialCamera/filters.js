/**
 * @fileoverview This file contains the filters that can be applied to an image.
 * @author Sebastian Romero
 */

/**
 * Represents an image filter interface. This class is meant to be extended by subclasses.
 */
class ImageFilter {
/**
 * Applies a filter to the given pixel data.
 * @param {Uint8Array} pixelData - The pixel data to apply the filter to. The pixel data gets modified in place.
 * @param {number} [width=null] - The width of the image. Defaults to null.
 * @param {number} [height=null] - The height of the image. Defaults to null.
 * @throws {Error} - Throws an error if the applyFilter method is not implemented.
 */
  applyFilter(pixelData, width = null, height = null) {
    throw new Error('applyFilter not implemented');
  }
}

/**
 * Represents a grayscale filter that converts an image to grayscale.
 * @extends ImageFilter
 */
class GrayScaleFilter extends ImageFilter {
    /**
     * Applies the grayscale filter to the given pixel data.
     * @param {Uint8ClampedArray} pixelData - The pixel data to apply the filter to.
     * @param {number} [width=null] - The width of the image.
     * @param {number} [height=null] - The height of the image.
     */
    applyFilter(pixelData, width = null, height = null) {
        for (let i = 0; i < pixelData.length; i += 4) {
            const r = pixelData[i];
            const g = pixelData[i + 1];
            const b = pixelData[i + 2];
            const gray = (r + g + b) / 3;
            pixelData[i] = gray;
            pixelData[i + 1] = gray;
            pixelData[i + 2] = gray;
        }
    }
}

/**
 * A class representing a black and white image filter.
 * @extends ImageFilter
 */
class BlackAndWhiteFilter extends ImageFilter {
    applyFilter(pixelData, width = null, height = null) {
        for (let i = 0; i < pixelData.length; i += 4) {
            const r = pixelData[i];
            const g = pixelData[i + 1];
            const b = pixelData[i + 2];
            const gray = (r + g + b) / 3;
            const bw = gray > 127 ? 255 : 0;
            pixelData[i] = bw;
            pixelData[i + 1] = bw;
            pixelData[i + 2] = bw;
        }
    }
}

/**
 * Represents a color filter that applies a sepia tone effect to an image.
 * @extends ImageFilter
 */
class SepiaColorFilter extends ImageFilter {
    applyFilter(pixelData, width = null, height = null) {
        for (let i = 0; i < pixelData.length; i += 4) {
            const r = pixelData[i];
            const g = pixelData[i + 1];
            const b = pixelData[i + 2];
            const gray = (r + g + b) / 3;
            pixelData[i] = gray + 100;
            pixelData[i + 1] = gray + 50;
            pixelData[i + 2] = gray;
        }
    }
}

/**
 * Represents a filter that applies a pixelation effect to an image.
 * @extends ImageFilter
 */
class PixelateFilter extends ImageFilter {

    constructor(blockSize = 8){
        super();
        this.blockSize = blockSize;
    }

    applyFilter(pixelData, width, height) {
        for (let y = 0; y < height; y += this.blockSize) {
            for (let x = 0; x < width; x += this.blockSize) {
                const blockAverage = this.getBlockAverage(x, y, width, height, pixelData, this.blockSize);

                // Set all pixels in the block to the calculated average color
                for (let blockY = 0; blockY < this.blockSize && y + blockY < height; blockY++) {
                    for (let blockX = 0; blockX < this.blockSize && x + blockX < width; blockX++) {
                        const pixelIndex = ((y + blockY) * width + (x + blockX)) * 4;
                        pixelData[pixelIndex] = blockAverage.red;
                        pixelData[pixelIndex + 1] = blockAverage.green;
                        pixelData[pixelIndex + 2] = blockAverage.blue;
                    }
                }
            }
        }
    }

    /**
     * Calculates the average RGB values of a block of pixels.
     * 
     * @param {number} x - The x-coordinate of the top-left corner of the block.
     * @param {number} y - The y-coordinate of the top-left corner of the block.
     * @param {number} width - The width of the image.
     * @param {number} height - The height of the image.
     * @param {Uint8ClampedArray} pixels - The array of pixel data.
     * @returns {Object} - An object containing the average red, green, and blue values.
     */
    getBlockAverage(x, y, width, height, pixels) {
        let totalRed = 0;
        let totalGreen = 0;
        let totalBlue = 0;
        const blockSizeSquared = this.blockSize * this.blockSize;

        for (let blockY = 0; blockY < this.blockSize && y + blockY < height; blockY++) {
            for (let blockX = 0; blockX < this.blockSize && x + blockX < width; blockX++) {
                const pixelIndex = ((y + blockY) * width + (x + blockX)) * 4;
                totalRed += pixels[pixelIndex];
                totalGreen += pixels[pixelIndex + 1];
                totalBlue += pixels[pixelIndex + 2];
            }
        }

        return {
            red: totalRed / blockSizeSquared,
            green: totalGreen / blockSizeSquared,
            blue: totalBlue / blockSizeSquared,
        };
    }

}

/**
 * Represents a filter that applies a blur effect to an image.
 * @extends ImageFilter
 */
class BlurFilter extends ImageFilter {
    constructor(radius = 8) {
        super();
        this.radius = radius;
    }

    applyFilter(pixelData, width, height) {
        for (let y = 0; y < height; y++) {
            for (let x = 0; x < width; x++) {
                const pixelIndex = (y * width + x) * 4;

                const averageColor = this.getAverageColor(x, y, width, height, pixelData, this.radius);
                pixelData[pixelIndex] = averageColor.red;
                pixelData[pixelIndex + 1] = averageColor.green;
                pixelData[pixelIndex + 2] = averageColor.blue;
            }
        }
    }

    /**
     * Calculates the average color of a rectangular region in an image.
     * 
     * @param {number} x - The x-coordinate of the top-left corner of the region.
     * @param {number} y - The y-coordinate of the top-left corner of the region.
     * @param {number} width - The width of the region.
     * @param {number} height - The height of the region.
     * @param {Uint8ClampedArray} pixels - The pixel data of the image.
     * @param {number} radius - The radius of the neighborhood to consider for each pixel.
     * @returns {object} - An object representing the average color of the region, with red, green, and blue components.
     */
    getAverageColor(x, y, width, height, pixels, radius) {
        let totalRed = 0;
        let totalGreen = 0;
        let totalBlue = 0;
        let pixelCount = 0;

        for (let offsetY = -radius; offsetY <= radius; offsetY++) {
            for (let offsetX = -radius; offsetX <= radius; offsetX++) {
                const neighborX = x + offsetX;
                const neighborY = y + offsetY;

                if (neighborX >= 0 && neighborX < width && neighborY >= 0 && neighborY < height) {
                    const pixelIndex = (neighborY * width + neighborX) * 4;
                    totalRed += pixels[pixelIndex];
                    totalGreen += pixels[pixelIndex + 1];
                    totalBlue += pixels[pixelIndex + 2];
                    pixelCount++;
                }
            }
        }

        return {
            red: totalRed / pixelCount,
            green: totalGreen / pixelCount,
            blue: totalBlue / pixelCount,
        };
    }
}
