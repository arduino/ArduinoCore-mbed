/**
 * @fileoverview This file contains classes that transform incoming data into higher-level data types.
 * @author Sebastian Romero
 */


/**
 * A transformer class that waits for a specific number of bytes before processing them.
 */
class BytesWaitTransformer {
    constructor(waitBytes = 1) {
        this.waitBytes = waitBytes;
        this.buffer = new Uint8Array(0);
        this.controller = undefined;
    }

    /**
     * Sets the number of bytes to wait before processing the data.
     * @param {number} waitBytes - The number of bytes to wait.
     */
    setBytesToWait(waitBytes) {
        this.waitBytes = waitBytes;
    }

    /**
     * Converts bytes into higher-level data types.
     * This method is meant to be overridden by subclasses.
     * @param {Uint8Array} bytes 
     * @returns 
     */
    convertBytes(bytes) {
        return bytes;
    }

    /**
     * Transforms the incoming chunk of data and enqueues the processed bytes to the controller.
     * It does so when the buffer contains at least the specified number of bytes.
     * @param {Uint8Array} chunk - The incoming chunk of data.
     * @param {TransformStreamDefaultController} controller - The controller for enqueuing processed bytes.
     * @returns {Promise<void>} - A promise that resolves when the transformation is complete.
     */
    async transform(chunk, controller) {
        this.controller = controller;

        // Concatenate incoming chunk with existing buffer
        this.buffer = new Uint8Array([...this.buffer, ...chunk]);

        while (this.buffer.length >= this.waitBytes) {
            // Extract the required number of bytes
            const bytesToProcess = this.buffer.slice(0, this.waitBytes);

            // Remove processed bytes from the buffer
            this.buffer = this.buffer.slice(this.waitBytes);

            // Notify the controller that bytes have been processed
            controller.enqueue(this.convertBytes(bytesToProcess));
        }
    }

    /**
     * Flushes the buffer and processes any remaining bytes when the stream is closed.
     * 
     * @param {WritableStreamDefaultController} controller - The controller for the writable stream.
     */
    flush(controller) {
        if (this.buffer.length > 0) {
            // Handle remaining bytes (if any) when the stream is closed
            const remainingBytes = this.buffer.slice();
            console.log("Remaining bytes:", remainingBytes);

            // Notify the controller that remaining bytes have been processed
            controller?.enqueue(remainingBytes);
        }
    }
}

/**
 * Represents an Image Data Transformer that converts bytes into image data.
 * See other example for PNGs here: https://github.com/mdn/dom-examples/blob/main/streams/png-transform-stream/png-transform-stream.js
 * @extends BytesWaitTransformer
 */
class ImageDataTransformer extends BytesWaitTransformer {
    /**
     * Creates a new instance of the Transformer class.
     * @param {CanvasRenderingContext2D} context - The canvas rendering context.
     * @param {number} [width=null] - The width of the image.
     * @param {number} [height=null] - The height of the image.
     * @param {string} [imageMode=null] - The image mode.
     */
    constructor(context, width = null, height = null, imageMode = null) {
        super();
        this.context = context;
        this.imageDataProcessor = new ImageDataProcessor();
        if (width && height){
            this.setResolution(width, height);
        }
        if (imageMode){
            this.setImageMode(imageMode);
        }
    }

    /**
     * Sets the resolution of the camera image that is being processed.
     * 
     * @param {number} width - The width of the resolution.
     * @param {number} height - The height of the resolution.
     */
    setResolution(width, height) {
        this.width = width;
        this.height = height;
        this.imageDataProcessor.setResolution(width, height);
        if(this.isConfigured()){
            this.setBytesToWait(this.imageDataProcessor.getTotalBytes());
        }
    }

    /**
     * Sets the image mode of the camera image that is being processed.
     * Possible values: RGB565, GRAYSCALE, RGB888, BAYER
     * 
     * @param {string} imageMode - The image mode to set.
     */
    setImageMode(imageMode) {
        this.imageMode = imageMode;
        this.imageDataProcessor.setImageMode(imageMode);
        if(this.isConfigured()){
            this.setBytesToWait(this.imageDataProcessor.getTotalBytes());
        }
    }

    /**
     * Checks if the image data processor is configured.
     * This is true if the image mode and resolution are set.
     * @returns {boolean} True if the image data processor is configured, false otherwise.
     */
    isConfigured() {
        return this.imageMode && this.width && this.height;
    }

    /**
     * Resets the state of the transformer.
     */
    reset() {
        this.imageMode = null;
        this.width = null;
        this.height = null;
        this.imageDataProcessor.reset();
    }

    /**
     * Converts the given raw bytes into an ImageData object by using the ImageDataProcessor.
     * 
     * @param {Uint8Array} bytes - The bytes to convert.
     * @returns {ImageData} The converted ImageData object.
     */
    convertBytes(bytes) {
        let pixelData = this.imageDataProcessor.convertToPixelData(bytes);
        
        if(this.filter){
            this.filter.applyFilter(pixelData, imageDataTransfomer.width, imageDataTransfomer.height);
        }

        const imageData = this.context.createImageData(imageDataTransfomer.width, imageDataTransfomer.height);
        imageData.data.set(pixelData);
        return imageData;
    }
}