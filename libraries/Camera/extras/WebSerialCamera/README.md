# 📹 WebSerial Camera Stream

This folder contains a web application that provides a camera stream over WebSerial.
This is an experimental feature not supported in all browsers. It's recommended to use Google Chrome.
See [Browser Compatibility](https://developer.mozilla.org/en-US/docs/Web/API/Web_Serial_API#browser_compatibility)

## Instructions

1. Upload the companion [Arduino sketch](../../examples/CameraCaptureWebSerial/CameraCaptureWebSerial.ino) to your board.
2. Open the web app either by directly opening the index.html file or serving it via a webserver and opening the URL provided by the webserver.
3. Click "Connect". Your board's serial port should show up in the popup. Select it. Click once again "Connect". The camera feed should start. If the board has been previously connected to the browser, it will connect automatically.
4. (Optional) click "Save Image" if you want to save individual camera frames to your computer.