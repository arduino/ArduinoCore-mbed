/**
 * This is a simple example of a web app that connects to an Arduino board
 * and reads the battery level and other battery related characteristics.
 * 
 * It uses the Web Bluetooth API to connect to the Arduino board.
 * 
 * Instructions:
 * 1. Upload the NiclaSenseME_BatteryStatus sketch to the Arduino board.
 * 2. Open the index.html file in a browser that supports the Web Bluetooth API (Chrome, Edge, Opera).
 * 3. Click on the Connect button to connect to the Arduino board.
 * 
 * Initial author: Sebastian Romero @sebromero
 */


/// UI elements
const connectButton = document.getElementById('connect');
const batteryLevelElement = document.getElementById('battery-level');
const batteryLabel = document.getElementById('battery-label');
const chargingIconElement = document.getElementById('charging-icon');
const externalPowerIconElement = document.getElementById('external-powered-icon');

const serviceUuid = '19b10000-0000-537e-4f6c-d104768a1214';
let pollIntervalID;
let peripheralDevice;

/// Data structure to hold the characteristics and their values plus data conversion functions.
let data = {
    "batteryPercentage": {
        "name": "Battery Percentage",
        "value": 0,
        "unit": "%",
        "characteristic": null,
        "characteristicUUID": "19b10000-1001-537e-4f6c-d104768a1214",
        "extractData": function(dataView) {
            return dataView.getInt8(0);
        }
    },
    "batteryVoltage": {
        "name": "Battery Voltage",
        "value": 0,
        "unit": "V",
        "characteristic": null,
        "characteristicUUID": "19b10000-1002-537e-4f6c-d104768a1214",
        "extractData": function(dataView) {
            return dataView.getFloat32(0, true);
        }
    },
    "batteryChargeLevel": {
        "name": "Battery Charge Level",
        "value": 0,
        "unit": "",
        "characteristic": null,
        "characteristicUUID": "19b10000-1003-537e-4f6c-d104768a1214",
        "extractData": function(dataView) {
            return dataView.getInt8(0);
        },
        "getColor": function(value) {
            // Red to green range with 5 steps and white for the unknown state
            const colors = ["#ffffff", "#ff2d2d", "#fc9228", "#ffea00", "#adfd5c", "#00c600"];
            return colors[value];
        }
    },
    "runsOnBattery": {
        "name": "Runs on Battery",
        "value": false,
        "unit": "",
        "characteristic": null,
        "characteristicUUID": "19b10000-1004-537e-4f6c-d104768a1214",
        "extractData": function(dataView) {
            return dataView.getUint8(0) == 1;
        }
    },

    "isCharging": {
        "name": "Is Charging",
        "value": false,
        "unit": "",
        "characteristic": null,
        "characteristicUUID": "19b10000-1005-537e-4f6c-d104768a1214",
        "extractData": function(dataView) {
            return dataView.getUint8(0) == 1;
        }
    }
};

function onDisconnected(event) {
    let device = event.target;
    connectButton.disabled = false;
    connectButton.style.opacity = 1;
    if(pollIntervalID) clearInterval(pollIntervalID);
    console.log(`Device ${device.name} is disconnected.`);

    // Reset the battery level display
    batteryLevelElement.style.width = "0px";
    batteryLabel.textContent = "";
    chargingIconElement.style.display = "none";
    externalPowerIconElement.style.display = "none";
}

/**
 * Connects to the Arduino board and starts reading the characteristics.
 * @param {Boolean} usePolling The default is to use notifications, but polling can be used instead.
 * In that case a poll interval can be defined.
 * @param {Number} pollInterval The interval in milliseconds to poll the characteristics from the device.
 */
async function connectToPeripheralDevice(usePolling = false, pollInterval = 5000){
    if (peripheralDevice && peripheralDevice.gatt.connected) {
        console.log("Already connected");
        return;
    }
   
    peripheralDevice = await navigator.bluetooth.requestDevice({
        filters: [{ services: [serviceUuid] }]
    });
    peripheralDevice.addEventListener('gattserverdisconnected', onDisconnected);

    const server = await peripheralDevice.gatt.connect();
    console.log("Connected to: " + peripheralDevice.name);
    const service = await server.getPrimaryService(serviceUuid);

    await Promise.all(
        Object.keys(data).map(async (key) => {
            let item = data[key];
            const characteristic = await service.getCharacteristic(item.characteristicUUID);
            item.characteristic = characteristic;
    
            if (!usePolling) {
                characteristic.addEventListener('characteristicvaluechanged', handleCharacteristicChange);
                characteristic.readValue(); // Perform an initial read
                await characteristic.startNotifications();
            }
        })
    );

    if (usePolling) {
        pollIntervalID = setInterval(readCharacteristicsData, pollInterval);
        await readCharacteristicsData();
    }
}

connectButton.addEventListener('click', async () => {
   try {
       await connectToPeripheralDevice();
       connectButton.disabled = true;
       connectButton.style.opacity = 0.5;
    } catch (error) {
        if(error.message === "User cancelled the requestDevice() chooser."){
           return;
        }
            
        console.error('Error:', error);
        connectButton.style.backgroundColor = "red";
    }
});

/**
 * Renders the data from the device in the UI.
 * It displays the battery level as a visual bar color coded from red to green.
 * It also displays the battery voltage and the percentage of the regulated voltage.
 * It also displays the charging and external power status.
 */
function displayBatteryData() {
    const batteryPercentage = data.batteryPercentage.value;
    const batteryVoltage = data.batteryVoltage.value;
    const regulatedVoltage = (batteryVoltage / batteryPercentage * 100).toFixed(2);
    
    // Map the range from 0-5 to 0-100
    const batteryPercentageMapped = data.batteryChargeLevel.value * 20;    
    batteryLevelElement.style.width = `${batteryPercentageMapped * 0.56}px`; // Scale the battery level to the width of the battery div
    batteryLevelElement.style.backgroundColor = data.batteryChargeLevel.getColor(data.batteryChargeLevel.value);
    batteryLabel.textContent = `${batteryVoltage.toFixed(2)}V (${batteryPercentage}% of ${regulatedVoltage}V)`;

    chargingIconElement.style.display = data.isCharging.value ? "block" : "none";
    externalPowerIconElement.style.display = data.runsOnBattery.value ? "none" : "block";
}

/**
 * Used together with polling to read the characteristics from the device.
 * After reading the data it is displayed in the UI by calling displayBatteryData().
 */
async function readCharacteristicsData() {
    await Promise.all(
        Object.keys(data).map(async (key) => {
            let item = data[key];
            console.log("Requesting " + item.name + "...");
            item.value = item.extractData(await item.characteristic.readValue());
            console.log(item.name + ": " + item.value + item.unit);
        })
    );
    displayBatteryData();
}

/**
 * Callback function that is called when a characteristic value changes.
 * Updates the data object with the new value and displays it in the UI by calling displayBatteryData().
 * @param {*} event The event that contains the characteristic that changed.
 */
function handleCharacteristicChange(event) {
    // Find the characteristic that changed in the data object by matching the UUID
    let dataItem = Object.values(data).find(item => item.characteristicUUID === event.target.uuid);    
    let dataView = event.target.value;
    dataItem.value = dataItem.extractData(dataView);

    console.log(`'${dataItem.name}' changed: ${dataItem.value}${dataItem.unit}`);
    displayBatteryData();
}
