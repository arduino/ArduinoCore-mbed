const connectButton = document.getElementById('connect');
const refreshButton = document.getElementById('refresh');
const startButton = document.getElementById('start');
const disconnectButton = document.getElementById('disconnect');
const canvas = document.getElementById('bitmapCanvas');
const ctx = canvas.getContext('2d');

const UserActionAbortError = 8;
const ArduinoUSBVendorId = 0x2341;

let config = {
  "RGB565": {
    "convert": convertRGB565ToRGB888,
    "bytesPerPixel": 2
  },
  "GRAYSCALE": {
    "convert": convertGrayScaleToRGB888,
    "bytesPerPixel": 1
  },
  "RGB888": {
    "convert": convertToRGB888,
    "bytesPerPixel": 3
  }
};

const imageWidth = 320; // Adjust this value based on your bitmap width
const imageHeight = 240; // Adjust this value based on your bitmap height
const mode = 'RGB565'; // Adjust this value based on your bitmap format
// const mode = 'GRAYSCALE'; // Adjust this value based on your bitmap format
const totalBytes = imageWidth * imageHeight * config[mode].bytesPerPixel;

// Set the buffer size to the total bytes. This allows to read the entire bitmap in one go.
const bufferSize = Math.min(totalBytes, 16 * 1024 * 1024); // Max buffer size is 16MB
const flowControl = 'hardware';
const baudRate = 115200; // Adjust this value based on your device's baud rate
const dataBits = 8; // Adjust this value based on your device's data bits
const stopBits = 2; // Adjust this value based on your device's stop bits

let currentPort, currentReader;

async function requestSerialPort(){
  try {
    // Request a serial port
    const port = await navigator.serial.requestPort({ filters: [{ usbVendorId: ArduinoUSBVendorId }] });
    currentPort = port;
    return port;
  } catch (error) {
    if(error.code != UserActionAbortError){
      console.log(error);
    }
    return null;
  }  
}

async function autoConnect(){
  if(currentPort){
    console.log('🔌 Already connected to a serial port.');
    return false;
  }

  // Get all serial ports the user has previously granted the website access to.
  const ports = await navigator.serial.getPorts();

  for (const port of ports) {    
    console.log('👀 Serial port found with VID: 0x' + port.getInfo().usbVendorId.toString(16));
    if(port.getInfo().usbVendorId === ArduinoUSBVendorId){
      currentPort = port;
      return await connectSerial(currentPort);
    }
  }
  return false;
}

async function connectSerial(port, baudRate = 115200, dataBits = 8, stopBits = 2, bufferSize = 4096) {
  try {
    // If the port is already open, close it
    if (port.readable) await port.close();
    await port.open({ baudRate: baudRate, parity: "even", dataBits: dataBits, stopBits: stopBits, bufferSize: bufferSize });
    console.log('✅ Connected to serial port.');
    return true;
  } catch (error) {
    return false;
  }
}


async function readBytes(port, numBytes, timeout = null){
  if(port.readable.locked){
    console.log('🔒 Stream is already locked. Ignoring request...');
    return null;
  }

  const bytesRead = new Uint8Array(numBytes);
  let bytesReadIdx = 0;
  let keepReading = true;  

  // As long as the errors are non-fatal, a new ReadableStream is created automatically and hence port.readable is non-null. 
  // If a fatal error occurs, such as the serial device being removed, then port.readable becomes null.

  while (port.readable && keepReading) {
    const reader = port.readable.getReader();
    currentReader = reader;
    let timeoutID = null;
    // let count = 0;

    try {      
      while (bytesReadIdx < numBytes) {
        if(timeout){
          timeoutID = setTimeout(() => {
            console.log('⌛️ Timeout occurred while reading.');
            if(port.readable) reader?.cancel();
          }, timeout);
        }

        const { value, done } = await reader.read();
        if(timeoutID) clearTimeout(timeoutID);

        if(value){
          for (const byte of value) {
            bytesRead[bytesReadIdx++] = byte;
            if (bytesReadIdx >= numBytes) break;
          }
          // count += value.byteLength;
          // console.log(`Read ${value.byteLength} (Total: ${count}) out of ${numBytes} bytes.}`);
        }

        if (done) {
          // |reader| has been canceled.
          console.log('🚫 Reader has been canceled');
          break;
        }
      }
      
    } catch (error) {
      // Handle |error|...
      console.log('💣 Error occurred while reading: ');
      console.log(error);
    } finally {
      keepReading = false;
      // console.log('🔓 Releasing reader lock...');
      reader?.releaseLock();
      currentReader = null;
    }
  }  
  return bytesRead;
}

// Get the pixel value using big endian
// Big-endian: the most significant byte comes first
function getPixelValue(data, index, bytesPerPixel){
  if(bytesPerPixel == 1){
    return data[index];
  } else if(bytesPerPixel == 2){
    return (data[index] << 8) | data[index + 1];
  } else if(bytesPerPixel == 3){
    return (data[index] << 16) | (data[index + 1] << 8) | data[index + 2];
  } else if(bytesPerPixel == 4){
    return (data[index] << 24) | (data[index + 1] << 16) | (data[index + 2] << 8) | data[index + 3];
  }

  return 0;
}

function renderBitmap(bytes, width, height) {
  canvas.width = width;
  canvas.height = height;
  const bytesPerPixel = config[mode].bytesPerPixel;
  const BYTES_PER_ROW = width * bytesPerPixel;

  const imageData = ctx.createImageData(canvas.width, canvas.height);
  const dataContainer = imageData.data;

  for (let row = 0; row < height; row++) {
    for (let col = 0; col < width; col++) {
      const sourceDataIndex = (row * BYTES_PER_ROW) + (col * bytesPerPixel);
      const pixelValue = getPixelValue(bytes, sourceDataIndex, bytesPerPixel);
      const [r, g, b] = config[mode].convert(pixelValue);

      const pixelIndex = ((row * width) + col) * 4;
      dataContainer[pixelIndex] = r; // Red channel
      dataContainer[pixelIndex + 1] = g; // Green channel
      dataContainer[pixelIndex + 2] = b; // Blue channel
      dataContainer[pixelIndex + 3] = 255; // Alpha channel (opacity)
    }
  }
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.putImageData(imageData, 0, 0);
}

async function requestFrame(port){
  if(!port?.writable) {
    console.log('🚫 Port is not writable. Ignoring request...');
    return;
  }
  // console.log('Writing 1 to the serial port...');
  // Write a 1 to the serial port
  const writer = port.writable.getWriter();
  await writer.write(new Uint8Array([1]));
  await writer.close();
}
async function renderStream(){
  while(true && currentPort){
    await renderFrame(currentPort);
  }
}

async function renderFrame(port){
  if(!port) return;
  const bytes = await getFrame(port);
  if(!bytes) return false; // Nothing to render
  // console.log(`Reading done ✅. Rendering image...`);
  // Render the bytes as a grayscale bitmap
  renderBitmap(bytes, imageWidth, imageHeight);
  return true;
}

async function getFrame(port) {
  if(!port) return;

  await requestFrame(port);
  // console.log(`Trying to read ${totalBytes} bytes...`);
  // Read the given amount of bytes
  return await readBytes(port, totalBytes, 2000);
}

async function disconnectSerial(port) {
  if(!port) return;
  try {
    currentPort = null;    
    await currentReader?.cancel();
    await port.close();
    console.log('🔌 Disconnected from serial port.');
  } catch (error) {
    console.error('💣 Error occurred while disconnecting: ' + error.message);
  };
}

startButton.addEventListener('click', renderStream);
connectButton.addEventListener('click', async () => { 
  currentPort = await requestSerialPort();
  if(await connectSerial(currentPort, baudRate, dataBits, stopBits, bufferSize, flowControl)){
    renderStream();
  }
});
disconnectButton.addEventListener('click', () => disconnectSerial(currentPort));
refreshButton.addEventListener('click', () => {
  renderFrame(currentPort);
});

navigator.serial.addEventListener("connect", (e) => {
  // Connect to `e.target` or add it to a list of available ports.
  console.log('🔌 Serial port became available. VID: 0x' + e.target.getInfo().usbVendorId.toString(16));
  autoConnect().then((connected) => {
    if(connected){
      renderStream();
    };
  });
});

navigator.serial.addEventListener("disconnect", (e) => {
  // Remove `e.target` from the list of available ports.
  console.log('❌ Serial port lost. VID: 0x' + e.target.getInfo().usbVendorId.toString(16));
  currentPort = null;  
});

// On page load event, try to connect to the serial port
window.addEventListener('load', async () => {
  console.log('🚀 Page loaded. Trying to connect to serial port...');
  setTimeout(() => {
    autoConnect().then((connected) => {
      if (connected) {
        renderStream();
      };
    });
  }, 1000);
});

if (!("serial" in navigator)) {
  alert("The Web Serial API is not supported in your browser.");
}