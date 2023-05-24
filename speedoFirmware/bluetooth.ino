
#define SERVICE_UUID "a5f125c0-7cec-4334-9214-58cffb8706c0"
#define CHARACTERISTIC_UUID_RX "a5f125c2-7cec-4334-9214-58cffb8706c0"
#define CHARACTERISTIC_UUID_TX "a5f125c1-7cec-4334-9214-58cffb8706c0"

BLECharacteristic speedoRxCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_NOTIFY);
BLECharacteristic speedoTxCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_WRITE);


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    speedoRxCharacteristic.setValue("connectSuccess");
    speedoRxCharacteristic.notify();
  }
  void onDisconnect(BLEServer* pServer) {
    pwOk = false;
    currentMenu = &mainMenu
    updateDisplay();
  }
};

//------------------------BLUETOOTH_CALLBACK---------------------------------------------------------------------------
class incomingCallbackHandler : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* speedoTxCharacteristic) {
    char* incomingMessage = (char*)speedoTxCharacteristic->getValue().c_str();
    strcat(incomingMessage, "\0");
    Serial.print("message received: ");
    char* messagePart;
    char delimiter[] = ",";
    messagePart = strtok(incomingMessage, ",");
    if (strcmp(messagePart, "#pw") == 0) {
      messagePart = strtok(NULL, delimiter);
      if (strcmp(messagePart, password) == 0) {
        pwOk = true;
        speedoRxCharacteristic.setValue("pwOk");
        speedoRxCharacteristic.notify();
      }
    } else if (pwOk) {
      if (strcmp(messagePart, "#kmstand") == 0) {
        kmstand = strtoull(messagePart, NULL, 10);
        speedoRxCharacteristic.setValue("success");
        speedoRxCharacteristic.notify();
      } else if(strcmp(messagePart, "#pulsesPerKm") == 0) {
        pulsesPerKm = strtoull(messagePart, NULL, 10);
        speedoRxCharacteristic.setValue("success");
        speedoRxCharacteristic.notify();
      } else {
        speedoRxCharacteristic.setValue("failedAuth");
        speedoRxCharacteristic.notify();
      }
    }
  }
};

void activateBLE() {
  // Create the BLE Device
  BLEDevice::init("speedo");

  // Create the BLE Server
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService* comService = pServer->createService(SERVICE_UUID);

  comService->addCharacteristic(&speedoTxCharacteristic);
  comService->addCharacteristic(&speedoRxCharacteristic);
  speedoRxCharacteristic.addDescriptor(new BLE2902());
  speedoTxCharacteristic.setCallbacks(new incomingCallbackHandler());
  // Start the service
  comService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}