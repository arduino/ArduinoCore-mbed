class BytesWaitTransformer {
    constructor(waitBytes = 1) {
        this.waitBytes = waitBytes;
        this.buffer = new Uint8Array(0);
        this.controller = undefined;
    }

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

    flush(controller) {
        if (this.buffer.length > 0) {
            // Handle remaining bytes (if any) when the stream is closed
            const remainingBytes = this.buffer.slice();
            console.log("Remaining bytes:", remainingBytes);

            // Notify the controller that remaining bytes have been processed
            controller.enqueue(remainingBytes);
        }
    }
}

class ImageDataTransformer extends BytesWaitTransformer {
    constructor(context, width, height, imageMode) {
        super(1);
        this.width = width;
        this.height = height;
    }

    setResolution(width, height) {
        this.width = width;
        this.height = height;
    }

    setImageMode(imageMode) {
        this.imageMode = imageMode;
    }

    convertBytes(bytes) {
        console.log("Converting bytes");
        let a = new Uint8Array(bytes);
        // Iterate over UInt8Array
        for (let i = 0; i < a.length; i++) {
            a[i] = a[i] * 2;
        }

        // const imageData = new ImageData(this.width, this.height);
        // for (let i = 0; i < bytes.length; i++) {
        //     imageData.data[i] = bytes[i];
        // }
        // return imageData;
        return bytes;
    }
}