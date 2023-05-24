

//---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  //start and check fram (sda, scl)
  Wire.begin(21, 22);
  uint16_t test = fram.read8(0x50);
  if (test != 0) {
    Serial.println("Fram read sucessfull");
  }
  kmstand = fram.read32(kmstandAddress);
  uint32_t temp = fram.read32(kmstandBackupAddress);
  if (kmstand != temp) {
    Serial.println("Kmstand read inconsistent");
  } else {
    Serial.print("KM stand: ");
    Serial.println(kmstand);
  }
  //read pulses per km
  pulsesPerKm = fram.read16(pulsesPerKmAddress);
  temp = 0;
  temp = fram.read16(pulsesPerKmBackupAddress);
  if (pulsesPerKm != temp) {
    Serial.println("pulses per KM read inconsistent");
  } else {
    Serial.print("pulses Per KM: ");
    Serial.println(pulsesPerKm);
  }
  if (pulsesPerKm <= 0) {
    pulsesPerKm = 4971;
  }

  //read pulses per km
  pulsecounter = fram.read16(pulsesAddress);
  temp = 0;
  temp = fram.read16(pulsesBackupAddress);
  if (pulsecounter != temp) {
    Serial.println("current pulse count read inconsistent");
  } else {
    Serial.print("current pulsecount: ");
    Serial.println(pulsecounter);
  }

  //Here a new LedController object is created without hardware SPI.
  lc = LedController<2, 1>(DIN, CLK, CS);

  /* Set the brightness to a medium values */
  lc.setIntensity(brightness);
  /* and clear the display */
  //updateDisplay(0, (byte*)10);
  kmstand = 1234;
  Serial.println(kph);
  itoa(kph,kphString,10);
  analogReadResolution(4);
  getBrightness();
  ledcSetup(0, 1000, 4);
  ledcAttachPin(kphLedPin, 0);
  ledcWrite(0, brightness);
  updateDisplay();
  pinMode(btnPin, INPUT_PULLUP);
  Serial.print(currentMenu->items[currentMenu->selected].segment1);
  Serial.print(*currentMenu->items[currentMenu->selected].segment2);
  xTaskCreatePinnedToCore(mainTask, "mainTask", 5000, NULL, 1, &Task1, 0);
  //delay(500);  // needed to start-up task1
  xTaskCreatePinnedToCore(barLedTask, "barLedTask", 5000, NULL, 1, &Task2, 1);
}


void mainTask(void* parameter) {
  while (1) {
    if (longpress) {
      Serial.println("longpress detected");
      if (currentMenu->items[currentMenu->selected].subMenu != NULL) {
        // if there is a submenu, change to submenu
        currentMenu = currentMenu->items[currentMenu->selected].subMenu;
      } else if (currentMenu->items[currentMenu->selected].action != NULL) {
        // if there is an action, setup current action and current parameter
        currentMenu->items[currentMenu->selected].action(currentMenu->items[currentMenu->selected].param);
      } else if (currentMenu->parentMenu != NULL) {
        // if there is a paremnt menu, change to the parent menu
        currentMenu = currentMenu->parentMenu;
      } else {
        // if everything else fails, switch to main menu
        currentMenu = &mainMenu;
      }
      longpress = 0;
      updateDisplay();
    }
    if(shortpress){
      Serial.println("shortpress detected");
        currentMenu->selected = currentMenu->selected + 1;
        if(currentMenu->selected >= currentMenu->numItems){
          currentMenu->selected = 0;
        }
        shortpress = 0;
        updateDisplay();
    }

    if (pulsecounter >= pulsesPerKm) {
      kmstand++;
      updateDisplay();
      pulsecounter = pulsecounter - pulsesPerKm;
    }
    lastKph = kph;
    if (pulseTime[2] != 0) {
      int onePulseTime = 0;
      for (int i = 0; i < 3; i++) {
        onePulseTime = onePulseTime + pulseTime[i];
      }
      onePulseTime = onePulseTime / 3;
      kph = 3600000000 / (onePulseTime * pulsesPerKm);
    } else {
      kph = 0;
    }
    if (kph != lastKph) {
      itoa(kph, kphString, 10);
      updateDisplay();
    }
    delay(mainTaskDelay);
  }
}

void barLedTask(void* parameter) {
  for (int i = 0; i < 7; i++) {
    pinMode(columnPin[i], OUTPUT);
    digitalWrite(columnPin[i], LOW);
  }
  for (int i = 0; i < 3; i++) {
    pinMode(rowPin[i], OUTPUT);
    digitalWrite(rowPin[i], LOW);
  }
  attachInterrupt(vssPin, signalCounter, FALLING);
  attachInterrupt(btnPin, btnTrigger, CHANGE);
  while (1) {
    leds = kph / 10 + 1;
    columns = leds / 3;
    rows = leds % 3;

    //clear leds
    for (int k = 0; k < 3; k++) {
      digitalWrite(rowPin[k], LOW);
    }
    for (int i = 0; i < 7; i++) {
      digitalWrite(columnPin[i], LOW);
    }
    //set new leds
    for (int i = 0; i < columns; i++) {
      digitalWrite(columnPin[i], HIGH);
    }
    for (int k = 0; k < 3; k++) {
      digitalWrite(rowPin[k], HIGH);
    }
    delay(delaytime);
    //switch
    for (int k = 0; k < 3; k++) {
      digitalWrite(rowPin[k], LOW);
    }
    for (int i = 0; i < columns; i++) {
      digitalWrite(columnPin[i], LOW);
    }
    for (int k = 0; k < rows; k++) {
      digitalWrite(rowPin[k], HIGH);
    }
    digitalWrite(columnPin[columns], HIGH);
    delay(delaytime);
  }
}

void loop() {
}