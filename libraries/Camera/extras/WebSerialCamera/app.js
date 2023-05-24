const connectButton = document.getElementById('connect');
const refreshButton = document.getElementById('refresh');
const startButton = document.getElementById('start');
const saveImageButton = document.getElementById('save-image');
const canvas = document.getElementById('bitmapCanvas');
const ctx = canvas.getContext('2d');

// TODO check for signals 
// TODO implement transformer
// TODO get image format from device
// SEE: https://developer.chrome.com/articles/serial/#transforming-streams
// SEE: https://developer.chrome.com/articles/serial/#signals

config = {
  "RGB565": {    
    "bytesPerPixel": 2
  },
  "GRAYSCALE": {    
    "bytesPerPixel": 1
  },
  "RGB888": {    
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

const imageDataProcessor = new ImageDataProcessor(ctx, mode, imageWidth, imageHeight);
const connectionHandler = new SerialConnectionHandler(baudRate, dataBits, stopBits, "even", "hardware", bufferSize);

connectionHandler.onConnect = () => {
  connectButton.textContent = 'Disconnect';
  renderStream();
};

connectionHandler.onDisconnect = () => {
  connectButton.textContent = 'Connect';
};

function renderBitmap(bytes, width, height) {
  canvas.width = width;
  canvas.height = height;
  const imageData = imageDataProcessor.getImageDataBytes(bytes, width, height);
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.putImageData(imageData, 0, 0);
}

async function renderStream(){
  while(connectionHandler.isConnected()){
    await renderFrame();
  }
}

async function renderFrame(){
  if(!connectionHandler.isConnected()) return;
  const bytes = await connectionHandler.getFrame(totalBytes);
  if(!bytes || bytes.length == 0) return false; // Nothing to render
  // console.log(`Reading done ✅. Rendering image...`);
  renderBitmap(bytes, imageWidth, imageHeight);
  return true;
}

startButton.addEventListener('click', renderStream);
connectButton.addEventListener('click', async () => { 
  if(connectionHandler.isConnected()){
    connectionHandler.disconnectSerial();
  } else {
    await connectionHandler.requestSerialPort();
    await connectionHandler.connectSerial();
  }
});
refreshButton.addEventListener('click', () => {
  renderFrame();
});

saveImageButton.addEventListener('click', () => {
  const link = document.createElement('a');
  link.download = 'image.png';
  link.href = canvas.toDataURL();
  link.click();
  link.remove();
});

// On page load event, try to connect to the serial port
window.addEventListener('load', async () => {
  console.log('🚀 Page loaded. Trying to connect to serial port...');
  setTimeout(() => {
    connectionHandler.autoConnect();
  }, 1000);
});

if (!("serial" in navigator)) {
  alert("The Web Serial API is not supported in your browser.");
}