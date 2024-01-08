/**
 * @fileoverview This file contains the main application logic.
 * @author Sebastian Romero
 */

const connectButton = document.getElementById('connect');
const refreshButton = document.getElementById('refresh');
const startButton = document.getElementById('start');
const saveImageButton = document.getElementById('save-image');
const canvas = document.getElementById('bitmapCanvas');
const ctx = canvas.getContext('2d');

// Check the following links for more information on the Web Serial API:
// https://developer.chrome.com/articles/serial/
// https://wicg.github.io/serial/


const imageDataProcessor = new ImageDataProcessor();
let imageDataTransfomer = new ImageDataTransformer();
const connectionHandler = new SerialConnectionHandler();

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
  imageDataTransfomer.setImageMode(imageMode);
  imageDataTransfomer.setResolution(imageResolution.width, imageResolution.height);
  connectionHandler.setTransformer(imageDataTransfomer);
  renderStream();
};

connectionHandler.onDisconnect = () => {
  connectButton.textContent = 'Connect';
  imageDataProcessor.reset();
};

function renderBitmap(width, height, imageData) {
  canvas.width = width;
  canvas.height = height;
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
  const imageData = ctx.createImageData(320, 240);
  const data = imageDataProcessor.getImageData(bytes);
  imageData.data.set(data);

  renderBitmap(imageDataProcessor.width, imageDataProcessor.height, imageData);
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