/**
 * @fileoverview This file contains the SerialConnectionHandler class.
 * It handles the connection between the browser and the Arduino board via Web Serial.
 * @author Sebastian Romero
 */

const ArduinoUSBVendorId = 0x2341;
const UserActionAbortError = 8;

/**
 * Handles the connection between the browser and the Arduino board via Web Serial.
 * Please note that for board with software serial over USB, the baud rate and other serial settings have no effect.
 */
class SerialConnectionHandler {
    /**
     * Represents a serial connection handler.
     * @constructor
     * @param {number} [baudRate=115200] - The baud rate of the serial connection.
     * @param {number} [dataBits=8] - The number of data bits.
     * @param {number} [stopBits=1] - The number of stop bits.
     * @param {string} [parity="none"] - The parity setting.
     * @param {string} [flowControl="none"] - The flow control setting.
     * @param {number} [bufferSize=2097152] - The size of the buffer in bytes. The default value is 2 MB.  Max buffer size is 16MB.
     * @param {number} [timeout=2000] - The connection timeout value in milliseconds. The default value is 2000 ms.
     */
    constructor(baudRate = 115200, dataBits = 8, stopBits = 1, parity = "none", flowControl = "none", bufferSize = 2 * 1024 * 1024, timeout = 2000) {
        this.baudRate = baudRate;
        this.dataBits = dataBits;
        this.stopBits = stopBits;
        this.flowControl = flowControl;
        this.bufferSize = bufferSize;
        this.parity = parity;
        this.timeout = timeout;
        this.currentPort = null;
        this.currentReader = null;
        this.currentTransformer = null;
        this.readableStreamClosed = null;
        this.registerEvents();
    }

    /**
     * Sets the connection timeout for the serial connection.
     * @param {number} timeout - The timeout value in milliseconds.
     */
    setConnectionTimeout(timeout) {
        this.timeout = timeout;
    }

    /**
     * Prompts the user to select a serial port.
     * @returns {Promise<SerialPort>} The serial port that the user has selected.
     */
    async requestSerialPort() {
        try {
            const port = await navigator.serial.requestPort({ filters: [{ usbVendorId: ArduinoUSBVendorId }] });
            this.currentPort = port;
            return port;
        } catch (error) {
            if (error.code != UserActionAbortError) {
                console.log(error);
            }
            return null;
        }
    }

    /**
     * Checks if the browser is connected to a serial port.
     * @returns {boolean} True if the browser is connected, false otherwise.
     */
    isConnected() {
        return this.currentPort?.readable != null;
    }

    /**
     * Opens a connection to the given serial port by using the settings specified in the constructor. 
     * If the port is already open, it will be closed first.
     * This method will call the `onConnect` callback before it returns.
     * @returns {boolean} True if the connection was successfully opened, false otherwise.
     */
    async connectSerial() {
        try {
            // If the port is already open, close it
            if (this.isConnected()) await this.currentPort.close();
            await this.currentPort.open({
                baudRate: this.baudRate,
                parity: this.parity,
                dataBits: this.dataBits,
                stopBits: this.stopBits,
                bufferSize: this.bufferSize,
                flowControl: this.flowControl
            });
            console.log('✅ Connected to serial port.');
            if(this.onConnect) this.onConnect();
            return true;
        } catch (error) {
            return false;
        }
    }

    /**
     * Disconnects from the current serial port. 
     * If a reading operation is in progress, it will be canceled.
     * This function will call the `onDisconnect` callback before it returns.
     * @returns {Promise<void>} A promise that resolves when the port has been closed.
     */
    async disconnectSerial() {
        if (!this.currentPort) return;
        try {
            const port = this.currentPort;
            this.currentPort = null;
            await this.currentReader?.cancel();
            await this.readableStreamClosed.catch(() => { }); // Ignores the error
            this.currentTransformer?.flush();
            await port.close();
            console.log('🔌 Disconnected from serial port.');
            if(this.onDisconnect) this.onDisconnect();
        } catch (error) {
            console.error('💣 Error occurred while disconnecting: ' + error.message);
        };
    }

    /**
     * Tries to establish a connection to the first available serial port that has the Arduino USB vendor ID.
     * This only works if the user has previously granted the website access to that serial port.
     * @returns {Promise<boolean>} True if the connection was successfully opened, false otherwise.
     */
    async autoConnect() {
        if (this.currentPort) {
            console.log('🔌 Already connected to a serial port.');
            return false;
        }

        // Get all serial ports the user has previously granted the website access to.
        const ports = await navigator.serial.getPorts();

        for (const port of ports) {
            console.log('👀 Serial port found with VID: 0x' + port.getInfo().usbVendorId.toString(16));
            if (port.getInfo().usbVendorId === ArduinoUSBVendorId) {
                this.currentPort = port;
                return await this.connectSerial(this.currentPort);
            }
        }
        return false;
    }


    /**
     * Reads a specified number of bytes from the serial connection.
     * @param {number} numBytes - The number of bytes to read.
     * @returns {Promise<Uint8Array>} - A promise that resolves to a Uint8Array containing the read bytes.
     */
    async readBytes(numBytes) {
        return await this.readData(new BytesWaitTransformer(numBytes));
    }

    /**
     * Reads the specified number of bytes from the serial port.
     * @param {Transformer} transformer The transformer that is used to process the bytes.     
     * If the timeout is reached, the reader will be canceled and the read lock will be released.
     */
    async readData(transformer) {
        if(!transformer) throw new Error('Transformer is null');
        if(!this.currentPort) return null;
        if(this.currentPort.readable.locked) {
            console.log('🔒 Stream is already locked. Ignoring request...');
            return null;
        }
        
        const transformStream = new TransformStream(transformer);
        this.currentTransformer = transformer;
        // pipeThrough() cannot be used because we need a promise that resolves when the stream is closed
        // to be able to close the port. pipeTo() returns such a promise.
        // SEE: https://stackoverflow.com/questions/71262432/how-can-i-close-a-web-serial-port-that-ive-piped-through-a-transformstream
        this.readableStreamClosed = this.currentPort.readable.pipeTo(transformStream.writable);
        const reader = transformStream.readable.getReader();
        this.currentReader = reader;
        let timeoutID = null;

        try {
            if (this.timeout) {
                timeoutID = setTimeout(() => {
                    console.log('⌛️ Timeout occurred while reading.');
                    if (this.currentPort?.readable) reader?.cancel();
                    this.currentTransformer.flush();
                }, this.timeout);
            }
            const { value, done } = await reader.read();
            if (timeoutID) clearTimeout(timeoutID);

            if (done) {
                console.log('🚫 Reader has been canceled');
                return null;
            }
            return value;
        } catch (error) {
            console.error('💣 Error occurred while reading: ' + error.message);
        } finally {
            // console.log('🔓 Releasing reader lock...');
            await reader?.cancel(); // Discards any enqueued data
            await this.readableStreamClosed.catch(() => { }); // Ignores the error
            reader?.releaseLock();
            this.currentReader = null;
            this.currentTransformer = null;
        }
    }

    /**
     * Sends the provided byte array data through the current serial port.
     * 
     * @param {ArrayBuffer} byteArray - The byte array data to send.
     * @returns {Promise<void>} - A promise that resolves when the data has been sent.
     */
    async sendData(byteArray) {
        if (!this.currentPort?.writable) {
            console.log('🚫 Port is not writable. Ignoring request...');
            return;
        }
        const writer = this.currentPort.writable.getWriter();
        await writer.write(new Uint8Array(byteArray));
        await writer.close();
    }

    /**
     * Reqests an image frame from the Arduino board by writing a 1 to the serial port.
     * @returns {Promise<void>} A promise that resolves when the frame has been requested and the write stream has been closed.
     */
    async requestFrame() {
        // console.log('Writing 1 to the serial port...');
        // Write a 1 to the serial port
        return this.sendData([1]);
    }

    /**
     * Requests the camera configuration from the board by writing a 2 to the serial port.
     * @returns {Promise} A promise that resolves with the configuration data.
     */
    async requestConfig() {
        return this.sendData([2]);
    }

    /**
     * Requests the camera resolution from the board and reads it back from the serial port.
     * The configuration simply consists of two bytes: the mode and the resolution.
     * @returns {Promise<ArrayBuffer>} The raw configuration data as an ArrayBuffer.
     */
    async getConfig() {
        if (!this.currentPort) return;

        await this.requestConfig();
        // console.log(`Trying to read 2 bytes...`);
        return await this.readBytes(2, this.timeout);
    }

    /**
     * Requests a frame from the Arduino board and reads the specified number of bytes from the serial port afterwards.
     * Times out after the timeout in milliseconds specified in the constructor.
     * @param {Transformer} transformer The transformer that is used to process the bytes.     
     */
    async getFrame(transformer) {
        if (!this.currentPort) return;
        await this.requestFrame();
        return await this.readData(transformer, this.timeout);
    }

    /**
     * Registers event listeners for the `connect` and `disconnect` events of the serial port.
     * The `connect` event is fired when a serial port becomes available not when it is opened.
     * When the `connect` event is fired, `autoConnect()` is called.
     * The `disconnect` event is fired when a serial port is lost.
     * When the `disconnect` event is fired, the `onDisconnect` callback is called.
     **/
    registerEvents() {
        navigator.serial.addEventListener("connect", (e) => {
            // Connect to `e.target` or add it to a list of available ports.
            console.log('🔌 Serial port became available. VID: 0x' + e.target.getInfo().usbVendorId.toString(16));
            this.autoConnect();
        });

        navigator.serial.addEventListener("disconnect", (e) => {
            console.log('❌ Serial port lost. VID: 0x' + e.target.getInfo().usbVendorId.toString(16));
            this.currentPort = null;
            if(this.onDisconnect) this.onDisconnect();
        });
    }
}