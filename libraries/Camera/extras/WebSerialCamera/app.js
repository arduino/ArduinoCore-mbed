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


// Set the buffer size to the total bytes. This allows to read the entire bitmap in one go.
const bufferSize = 1024 * 1024;//Math.min(totalBytes, 16 * 1024 * 1024); // Max buffer size is 16MB
const flowControl = 'hardware';
const baudRate = 115200; // Adjust this value based on your device's baud rate
const dataBits = 8; // Adjust this value based on your device's data bits
const stopBits = 2; // Adjust this value based on your device's stop bits

const imageDataProcessor = new ImageDataProcessor(ctx);
const connectionHandler = new SerialConnectionHandler(baudRate, dataBits, stopBits, "even", "hardware", bufferSize);

connectionHandler.onConnect = async () => {
  connectButton.textContent = 'Disconnect';
  cameraConfig = await connectionHandler.getConfig();
  if(!cameraConfig){
    console.error('🚫 Could not read camera configuration. Aborting...');
    return;
  }
  const imageMode = CAMERA_MODES[cameraConfig[0]];
  const imageResolution = CAMERA_RESOLUTIONS[cameraConfig[1]];
  if(!imageMode || !imageResolution){
    console.error(`🚫 Invalid camera configuration: ${cameraConfig[0]}, ${cameraConfig[1]}. Aborting...`);
    return;
  }
  imageDataProcessor.setMode(imageMode);
  imageDataProcessor.setResolution(imageResolution.width, imageResolution.height);
  renderStream();
};

connectionHandler.onDisconnect = () => {
  connectButton.textContent = 'Connect';
  imageDataProcessor.reset();
};

function renderBitmap(imageData) {
  canvas.width = imageDataProcessor.width;
  canvas.height = imageDataProcessor.height;
  ctx.clearRect(0, 0, canvas.width, canvas.height);
  ctx.putImageData(imageData, 0, 0);
}

async function renderStream(){
  while(connectionHandler.isConnected()){
    if(imageDataProcessor.isConfigured()) await renderFrame();
  }
}

async function renderFrame(){
  if(!connectionHandler.isConnected()) return;
  const bytes = await connectionHandler.getFrame(imageDataProcessor.getTotalBytes());
  if(!bytes || bytes.length == 0) return false; // Nothing to render
  // console.log(`Reading done ✅. Rendering image...`);
  const imageData = imageDataProcessor.getImageData(bytes);
  renderBitmap(imageData);
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
  if(imageDataProcessor.isConfigured()) renderFrame();
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