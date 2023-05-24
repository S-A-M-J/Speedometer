
function connectToBle() {
  // Connect to a device by passing the service UUID
  blueTooth.connect("a5f125c0-7cec-4334-9214-58cffb8706c0", gotCharacteristics);
  console.log('trying to connect');
}

function disconnectBle() {
  blueTooth.disconnect();
}

// A function that will be called once got characteristics
function gotCharacteristics(error, characteristics) {
  console.log('looking for characteristics');
  if (error) {
    console.log('error: ', error);
  } else {
    console.log('characteristics: ', characteristics);
    console.log(characteristics.length);
    if (characteristics.length != 2) {
      return;
    }
    for (let i = 0; i < 2; i++) {
      if (characteristics[i].uuid == 'a5f125c1-7cec-4334-9214-58cffb8706c0') {
        blueToothTXCharacteristic = characteristics[i];
        console.log('detected first characteristic');
      }
      if (characteristics[i].uuid == 'a5f125c2-7cec-4334-9214-58cffb8706c0') {
        blueToothRXCharacteristic = characteristics[i];
        console.log('detected second characteristic');
      }
    }

    blueTooth.startNotifications(blueToothRXCharacteristic, gotValue, 'string');
    connectButton.hide();
    // Add a event handler when the device is disconnected
    blueTooth.onDisconnected(onDisconnected);
  }
}


// A function that will be called once got values
function gotValue(value) {
  console.log('value: ', value);
  let splitString = split(value, ',');
  newData = true;
  if (splitString[0] == 'pwOk') {//status string
    showPasswordParam();
  } else if (splitString[0] == 'authFailed'){
    statusTitle.style.color = 'red';
    statusTitle.textContent = "Authentication failed. Wrong password!"; 
  } else if (splitString[0] == 'sucess'){
    if(splitString[1] == 'odo'){
    statusTitle.style.color = 'green';
    statusTitle.textContent = "Odometer sucessfully updated"; 
    } else if(splitString[1] == 'ppKm'){
      statusTitle.style.color = 'green';
      statusTitle.textContent = "pulses per KM sucessfully updated"; 
    }
  }
  setTimeout(statusTitle.textContent = "", 10000);
}

function onDisconnected() {
  console.log('Device got disconnected.');
  connectButton.show();
  hidePasswordParam();
}

function sendData(data) {
  const inputValue = data;
  if (!("TextEncoder" in window)) {
    console.log("Sorry, this browser does not support TextEncoder...");
  }
  var enc = new TextEncoder(); // always utf-8
  blueToothTXCharacteristic.writeValue(enc.encode(inputValue));
}
