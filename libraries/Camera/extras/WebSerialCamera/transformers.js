/**
 * @fileoverview This file contains classes that transform incoming data into higher-level data types.
 * @author Sebastian Romero
 */


/**
 * Represents a transformer that processes incoming data between start and stop sequences.
 */
class StartStopSequenceTransformer {
    constructor(startSequence = null, stopSequence = null, expectedBytes = null) {
        this.startSequence = new Uint8Array(startSequence);
        this.stopSequence = new Uint8Array(stopSequence);
        this.expectedBytes = expectedBytes;
        this.buffer = new Uint8Array(0);
        this.controller = undefined;
        this.waitingForStart = true;
    }

    /**
     * Sets the start sequence for the received data.
     * This is used to disregard any data before the start sequence.
     * @param {Array<number>} startSequence - The start sequence as an array of numbers.
     */
    setStartSequence(startSequence) {
        this.startSequence = new Uint8Array(startSequence);
    }

    /**
     * Sets the stop sequence for the received data.
     * This is used to know when the data has finished being sent and should be processed.
     * @param {Array<number>} stopSequence - The stop sequence as an array of numbers.
     */
    setStopSequence(stopSequence) {
        this.stopSequence = new Uint8Array(stopSequence);
    }

    /**
     * Sets the expected number of bytes for the received data.
     * This is used to check if the number of bytes matches the expected amount
     * and discard the data if it doesn't.
     * 
     * @param {number} expectedBytes - The expected number of bytes.
     */
    setExpectedBytes(expectedBytes) {
        this.expectedBytes = expectedBytes;
    }

    /**
     * Transforms the incoming chunk of data and enqueues the processed bytes to the controller
     * between start and stop sequences.
     * 
     * @param {Uint8Array} chunk - The incoming chunk of data.
     * @param {TransformStreamDefaultController} controller - The controller for enqueuing processed bytes.
     * @returns {Promise<void>} - A promise that resolves when the transformation is complete.
     */
    async transform(chunk, controller) {
        this.controller = controller;

        // Concatenate incoming chunk with existing buffer
        this.buffer = new Uint8Array([...this.buffer, ...chunk]);
        let startIndex = 0;

        // Only process data if at least one start and stop sequence is present in the buffer
        const minimumRequiredBytes = Math.min(this.startSequence.length, this.stopSequence.length);

        while (this.buffer.length >= minimumRequiredBytes) {
            if (this.waitingForStart) {
                // Look for the start sequence
                startIndex = this.indexOfSequence(this.buffer, this.startSequence, startIndex);

                if (startIndex === -1) {
                    // No start sequence found, discard the buffer
                    this.buffer = new Uint8Array(0);
                    return;
                }

                // Remove bytes before the start sequence including the start sequence
                this.buffer = this.buffer.slice(startIndex + this.startSequence.length);
                startIndex = 0; // Reset startIndex after removing bytes
                this.waitingForStart = false;
            }

            // Look for the stop sequence
            const stopIndex = this.indexOfSequence(this.buffer, this.stopSequence, startIndex);

            if (stopIndex === -1) {
                // No stop sequence found, wait for more data
                return;
            }

            // Extract bytes between start and stop sequences
            const bytesToProcess = this.buffer.slice(startIndex, stopIndex);
            // Remove processed bytes from the buffer including the stop sequence.
            this.buffer = this.buffer.slice(stopIndex + this.stopSequence.length);

            // Check if the number of bytes matches the expected amount
            if (this.expectedBytes !== null && bytesToProcess.length !== this.expectedBytes) {
                // Skip processing the bytes, but keep the remaining data in the buffer
                console.error(`🚫 Expected ${this.expectedBytes} bytes, but got ${bytesToProcess.length} bytes instead. Dropping data.`);
                this.waitingForStart = true;
                return;
            }

            // Notify the controller that bytes have been processed
            controller.enqueue(this.convertBytes(bytesToProcess));
            this.waitingForStart = true;
        }
    }

    /**
     * Flushes the buffer and discards any remaining bytes when the stream is closed.
     * 
     * @param {WritableStreamDefaultController} controller - The controller for the writable stream.
     */
    flush(controller) {
        // Discard the remaining data in the buffer
        this.buffer = new Uint8Array(0);
    }


    /**
     * Finds the index of the given sequence in the buffer.
     * 
     * @param {Uint8Array} buffer - The buffer to search.
     * @param {Uint8Array} sequence - The sequence to find.
     * @param {number} startIndex - The index to start searching from.
     * @returns {number} - The index of the sequence in the buffer, or -1 if not found.
     */
    indexOfSequence(buffer, sequence, startIndex) {
        for (let i = startIndex; i <= buffer.length - sequence.length; i++) {
            if (this.isSubarray(buffer, sequence, i)) {
                return i;
            }
        }
        return -1;
    }

    /**
     * Checks if a subarray is present at a given index in the buffer.
     * 
     * @param {Uint8Array} buffer - The buffer to check.
     * @param {Uint8Array} subarray - The subarray to check.
     * @param {number} index - The index to start checking from.
     * @returns {boolean} - True if the subarray is present at the given index, false otherwise.
     */
    isSubarray(buffer, subarray, index) {
        for (let i = 0; i < subarray.length; i++) {
            if (buffer[index + i] !== subarray[i]) {
                return false;
            }
        }
        return true;
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
    
}


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
 * @extends StartStopSequenceTransformer
 */
class ImageDataTransformer extends StartStopSequenceTransformer {
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
            this.setExpectedBytes(this.imageDataProcessor.getTotalBytes());
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
        this.filter = null;
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