const ArduinoUSBVendorId = 0x2341;
const UserActionAbortError = 8;

class SerialConnectionHandler {
    constructor(baudRate = 115200, dataBits = 8, stopBits = 1, parity = "none", flowControl = "none", bufferSize = 4096, timeout = 2000) {
        this.baudRate = baudRate;
        this.dataBits = dataBits;
        this.stopBits = stopBits;
        this.flowControl = flowControl;
        this.bufferSize = bufferSize;
        this.parity = parity;
        this.timeout = timeout;
        this.currentPort = null;
        this.currentReader = null;
        this.registerEvents();
    }

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

    isConnected() {
        return this.currentPort?.readable != null;
    }

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

    async disconnectSerial() {
        if (!this.currentPort) return;
        try {
            const port = this.currentPort;
            this.currentPort = null;
            await this.currentReader?.cancel();
            await port.close();
            console.log('🔌 Disconnected from serial port.');
            if(this.onDisconnect) this.onDisconnect();
        } catch (error) {
            console.error('💣 Error occurred while disconnecting: ' + error.message);
        };
    }

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

    async readBytes(numBytes, timeout = null) {
        if (this.currentPort.readable.locked) {
            console.log('🔒 Stream is already locked. Ignoring request...');
            return null;
        }

        const bytesRead = new Uint8Array(numBytes);
        let bytesReadIdx = 0;
        let keepReading = true;

        // As long as the errors are non-fatal, a new ReadableStream is created automatically and hence port.readable is non-null. 
        // If a fatal error occurs, such as the serial device being removed, then port.readable becomes null.

        while (this.currentPort?.readable && keepReading) {
            const reader = this.currentPort.readable.getReader();
            this.currentReader = reader;
            let timeoutID = null;
            // let count = 0;

            try {
                while (bytesReadIdx < numBytes) {
                    if (timeout) {
                        timeoutID = setTimeout(() => {
                            console.log('⌛️ Timeout occurred while reading.');
                            if (this.currentPort?.readable) reader?.cancel();
                        }, timeout);
                    }

                    const { value, done } = await reader.read();
                    if (timeoutID) clearTimeout(timeoutID);

                    if (value) {
                        for (const byte of value) {
                            bytesRead[bytesReadIdx++] = byte;
                            if (bytesReadIdx >= numBytes) break;
                        }
                        // count += value.byteLength;
                        // console.log(`Read ${value.byteLength} (Total: ${count}) out of ${numBytes} bytes.}`);
                    }

                    if (done) {
                        console.log('🚫 Reader has been canceled');
                        break;
                    }
                }

            } catch (error) {
                console.log('💣 Error occurred while reading: ' + error.message);
            } finally {
                keepReading = false;
                // console.log('🔓 Releasing reader lock...');
                reader?.releaseLock();
                this.currentReader = null;
            }
        }
        return bytesRead;
    }

    async requestFrame() {
        if (!this.currentPort?.writable) {
            console.log('🚫 Port is not writable. Ignoring request...');
            return;
        }
        // console.log('Writing 1 to the serial port...');
        // Write a 1 to the serial port
        const writer = this.currentPort.writable.getWriter();
        await writer.write(new Uint8Array([1]));
        await writer.close();
    }

    async getFrame(totalBytes) {
        if (!this.currentPort) return;

        await this.requestFrame(this.currentPort);
        // console.log(`Trying to read ${totalBytes} bytes...`);
        // Read the given amount of bytes
        return await this.readBytes(totalBytes, this.timeout);
    }

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