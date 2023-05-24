/* Atmega328p based speedometer
    written by Samuel Jahn
    updated last:
    19.05.21
*/

#include "FRAM.h"
#include "SparkFun_Tlc5940.h"
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#define NUM_LEDS 23
#define DATA_PIN 8
#include <stdlib.h>
#include "LedControl.h"
// Define the array of leds
CRGB leds[NUM_LEDS];

//LedControl(dataPin,clockPin,csPin,numDevices)
LedControl lc = LedControl(11, 13, 10, 2);

FRAM fram;
uint32_t start;
uint32_t stop;

const static uint8_t buttonPin = 12;
const static uint8_t pulseInputPin = 2;
const static uint8_t dimPin = A0;

//don't use the next 10 adresses after these
const static uint8_t kmstandAddress = 0x60;
const static uint8_t kmstandAddressBackup = 0x70;
const static uint8_t pulsesAddress = 0x80;
const static uint8_t pulsesAddressBackup = 0x90;

//value from 0 to 4095

int r = 100;
int g = 100;
int b = 100;
int kph = 0;
char kphString[3] = {0, 0, 0};
int shortpresses = 0;
int longpresses = 0;
int dimPinValue = 0;
int mainmenu = 0;
int submenu = 0;
int selectedDigit = 0;

long kmstand = 100;
char kmstandString[6] = {0, 0, 0, 0, 0, 0};
uint32_t kmstandBackup = 0;
int pulsecounter = 0;
int pulsecounterBackup = 0;
int pulsesPerKm = 0;
int prevDimPin = 0;
int prevBarledsOn = 0;
int intensity = 0;
uint8_t dimFactorInt = 0;
long pulseTime = 0;
int brightness = 255;

float dimFactor = 0.3;

unsigned long timer = 0;
unsigned long last_timer = 0;
unsigned long debouncer_timer = 0;
unsigned long pressstart = 0;
unsigned long pulseTimer = 0;
unsigned long pulseTimerTemp = 0;
unsigned long digitTimer = 0;
unsigned long pressTimer = 0;

boolean displaychange = false;
boolean debouncer = false;
boolean pressactivated = false;
boolean longpress = false;
boolean shortpress = false;
boolean menuwait = false;
boolean menuactivated = false;
boolean ledsChanged = false;
boolean blinking = false;
int t2 = 0;
int t1 = 0;
unsigned int loopcounter = 0;
int tempInput = 0;



//--------------------------------------------------------------------------------------

void display_all(char kphData[3], char kmData[6], int intensity) {
  //Serial.print("kphdata ");
  //Serial.println(kphData);
  //Serial.println(kmData);
  int kphAre0 = 0;
  int kmAre0 = 0;
  if (kphData[0] == '\0') {
    kphData[0] = '0';
  }
  if (kmData[0] == '\0') {
    kmData[0] = '0';
  }
  ////Serial.println(kphData);
  //Serial.println(kmData);
  if (isDigit(kphData[0])) {
    int temp = atoi(kphData);
    if (temp < 100) {
      kphAre0++;
      if (temp < 10) {
        kphAre0++;
      }
    }
  }

  if (isDigit(kmData[0]) && selectedDigit == 0) {
    long longtemp = atol(kmData);
    if (longtemp < 100000) {
      kmAre0++;
      if (longtemp < 10000) {
        kmAre0++;
        if (longtemp < 1000) {
          kmAre0++;
          if (longtemp < 100) {
            kmAre0++;
            if (longtemp < 10) {
              kmAre0++;
            }
          }
        }
      }
    }
  } else {
    kmAre0 = 0;
  }
  //Serial.print(kmAre0);
  lc.setIntensity(0, intensity);
  lc.setIntensity(1, intensity);
  lc.clearDisplay(0);
  lc.clearDisplay(1);
  for (int i = 0; i < (3 - kphAre0); i++) {
    if (kphData[i] == 'O') {
      kphData[i] = 0;
    }
    lc.setChar(0, i + kphAre0, kphData[i], false);
    //Serial.println("id1");
  }
  for (int i = 0; i < (6 - kmAre0); i++) {
    if (kmData[i] == 'O') {
      kmData[i] = 0;
    }
    if (!(blinking && (6 - selectedDigit) == i)) {
      lc.setChar(1, i + kmAre0, kmData[i], false);
      //Serial.print("digit ");
      //Serial.print(i+kmAre0);
      //Serial.print(" value ");
      //Serial.println(kmData[i]);
    }
  }
  //delay(1);
  //kphString[0] = '\0';
  //kmstandString[0] = '\0';
}

//-------------------------------------------------------------------------------
void menu() {
  //Serial.print(mainmenu);
  //Serial.println(submenu);
  //check if nothing was entered into menu
  if (menuwait) {
    if ((timer - pressTimer) >= 10000 && timer > pressTimer) {
      menuactivated = false;
      shortpresses = 0;
      shortpress = false;
      longpress = false;
      menuwait = true;
      mainmenu = 0;
      submenu = 0;
      FastLED.clear();
      FastLED.show();
      Serial.println("menu deactivated");
      return 0;
    }
  }
  if (submenu == 0) {
    menuwait = true;
  } else {
    menuwait = false;
  }
  long ten = 100000;
  char temp [5];
  switch (mainmenu) {
    case 0:
      //Serial.println("menu 0");
      menuwait = true;
      kphString[0] = 'C';
      kphString[1] = 'A';
      kphString[2] = 'L';
      if (submenu == 0) {
        itoa(pulsesPerKm, kmstandString, 10);
        if (longpress) {
          submenu++;
        }
        if (shortpress) {
          mainmenu++;
          submenu = 0;
        }
      }
      if (submenu == 1) {
        Serial.println("cal mode activated");
        pulsecounterBackup = pulsecounter;
        pulsecounter = 0;
        for (int i = 0; i < 6; i++) {
          kmstandString[i] = 'O';
        }
        submenu++;
      }
      if (submenu == 2) {
        itoa(pulsecounter, kmstandString, 10);
        if (shortpress) {
          submenu++;
        }
      }
      if (submenu == 3) {
        pulsesPerKm = pulsecounter;
        fram.write16(pulsesAddress, pulsecounter);
        fram.write16(pulsesAddressBackup, pulsecounter);
        itoa(pulsecounter, kmstandString, 10);
        submenu = 0;
        pulsecounter = pulsecounterBackup;
      }
      break;
    case 1:
      //Serial.println("menu 1");
      if (submenu == 0)  {
        ltoa(kmstand, kmstandString, 10);
        if (longpress) {
          submenu++;
          longpress = false;
        }
        if (shortpress) {
          mainmenu = 0;
        }
      }
      if (submenu == 1) {
        Serial.println("odo modify mode");
        submenu++;
        shortpresses = 0;
        selectedDigit++;
        ltoa(kmstand, kmstandString, 10);
        while (kmstand < ten) {
          memcpy(temp, kmstandString, sizeof(kmstandString));
          sprintf(kmstandString, "%d%s", 0, temp);
          Serial.print(kmstandString );
          ten = ten / 10;
          if (kmstand <= 1) {
          }
        }
        tempInput = kmstandString[6 - selectedDigit] - '0';
      }
      if (submenu == 2) {
        //Serial.print("kmstandString ");
        //Serial.println(kmstandString);
        int input = 0;
        //Serial.println(tempInput);
        input = (shortpresses + tempInput) % 10;
        //Serial.println(input);
        kmstandString[6 - selectedDigit] = input + '0';
        //Serial.print("kmstandString adj ");
        //Serial.println(kmstandString);
        if (longpress) {
          selectedDigit++;
          tempInput = kmstandString[6 - selectedDigit] - '0';
          shortpresses = 0;
          kmstand = atol(kmstandString);
          if (selectedDigit - 1 > 5) {
            submenu++;
            selectedDigit = 0;
            kmstand = atol(kmstandString);
            ltoa(kmstand, kmstandString, 10);
            Serial.println("odo modify mode done");
          }
        }
      }
      if (submenu == 3) {
        fram.write32(kmstandAddress, kmstand);
        fram.write32(kmstandAddressBackup, kmstand);
        submenu = 0;
      }
      kphString[0] = 'O';
      kphString[1] = 'D';
      kphString[2] = 'O';
       //Serial.println(kphString);
      break;
  }
  display_all(kphString, kmstandString, intensity);
  shortpress = false;
  longpress = false;
}
//-------------------------------------------------------------------------------
void setup() {

  /*
    The MAX72XX is in power-saving mode on startup,
    we have to do a wakeup call
  */
  Serial.begin(9600);
  int devices = lc.getDeviceCount();
  lc.setScanLimit(0, 3); 
  lc.setScanLimit(0, 6); 
  for (int address = 0; address < devices; address++) {
    /*The MAX72XX is in power-saving mode on startup*/
    lc.shutdown(address, false);
    /* Set the brightness to a medium values */
    lc.setIntensity(address, 8);
    /* and clear the display */
    lc.clearDisplay(address);
  }

  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  FastLED.clear();
  //c;

  dimPinValue = analogRead(dimPin);
  if (dimPinValue > 150) {
    dimFactor = (float)dimPinValue / 1024;
  } else {
    dimFactor = 0.5;
  }
  brightness = round(254 * dimFactor);
  intensity = round(dimFactor * 15);
  dimFactorInt = round(255 * dimFactor);
  ledsChanged = true;


  //start and check fram
  Wire.begin();
  int rv = fram.begin(0x50);
  uint8_t test = fram.read8(0x50);
  if (test != 0) {
    leds[0] = CRGB::Red;
    FastLED.show();
    delay(1000);
    FastLED.clear();
    FastLED.show();
  }

  //read out kmstand from fram
  kmstand = fram.read32(kmstandAddress);
  //test
  if (kmstand <= 0) {
    kmstand = 12345;
  }
  kmstandBackup = fram.read32(kmstandAddressBackup);

  pulsesPerKm = fram.read16(pulsesAddress);
  if (pulsesPerKm <= 0) {
    pulsesPerKm = 4971;
  }
  int temp = fram.read16(pulsesAddressBackup);

  //compare normal value to backup value
  if (kmstand != kmstandBackup) {
    leds[1] = CRGB::Yellow;
    FastLED.show();
    delay(1000);
    FastLED.clear();
    FastLED.show();
  }

  if (temp != pulsesPerKm) {
    leds[2] = CRGB::Yellow;
    FastLED.show();
    delay(1000);
    FastLED.clear();
    FastLED.show();
  }

  //Cycling through at leds at beginning
  //FastLED.setBrightness(128);

  Serial.println(brightness);
  for (int i = 0; i <= 22; i++) {
    leds[i].setHSV(0, 0, (byte)brightness);
    FastLED.show();
    delay(200);
    leds[i] = CRGB::Black;
    FastLED.show();
  }
  FastLED.clear();
  FastLED.show();
  last_timer = millis();
  leds[21].setHSV(0, 0, (byte)brightness);
  leds[22].setHSV(0, 0, (byte)brightness);
  FastLED.show();
  delay(500);
  pinMode(12, INPUT_PULLUP);

  //kph = 1;
  //Interrupt catches signals from transmission
  attachInterrupt(digitalPinToInterrupt(pulseInputPin), pulsedetector, RISING);



  Serial.println("setup complete");
  Serial.println(kmstand);
  //Serial.println(dimFactor);

}

//-----------------------------------------------------------------------------------

void loop() {
  //Serial.println("loop is going");
  loopcounter++;
  if (loopcounter % 20 == 0) {
    if (blinking) {
      blinking = false;
    } else {
      blinking = true;
    }
    //kmstand++;
    //kph++;
  }
  timer = millis();

  //check for rollover with millis function
  if (last_timer >= timer) {
    last_timer = timer;
    timer = millis();
  }

  //button debouncer
  if (debouncer) {
    //Serial.println("debouncer active");
    if (debouncer_timer == 0) {
      debouncer_timer = timer;
    }
    if (timer - debouncer_timer > 200) {
      debouncer = false;
      debouncer_timer = 0;
    }
  }

  //check button presses
  if (!debouncer) {
    if (!digitalRead(buttonPin)) {
      if (!pressactivated) {
        pressactivated = true;
        pressstart = timer;
        debouncer = true;
      }
    }
    else if (pressactivated) {
      FastLED.clear();
      last_timer = millis();
      leds[1].setHSV(0, 0, (byte)brightness);
      FastLED.show();
      if (timer - pressstart > 800) {
        debouncer = true;
        longpress = true;
        Serial.println("longpress");
      } else {
        debouncer = true;
        shortpress = true;
        shortpresses++;


        Serial.println("shortpress");
      }
      pressactivated = false;
      pressTimer = millis();
    }
  }

  //menu
  if (shortpress && !menuactivated) {

    menuactivated = true;
    shortpress = false;
  }

  if (menuactivated) {
    menu();
    //Serial.println("menu active");
    ledsChanged = true;
    return 0;
  }

  //pulseTime = 100;
  //read out kph
  if (pulseTime > 0 && pulseTime < 1000) {
    //Serial.println(pulseTime);
    // Serial.println(pulsesPerKm);
    long temp = pulseTime * pulsesPerKm;
    //Serial.println(temp);
    kph = round(3600000 / temp);
    //Serial.print("kph: ");
    //Serial.println(kph);
  } else {
    kph = 0;
    //Serial.println("kph: reset ");
  }

  if (timer - pulseTimerTemp >= 900) {
    pulseTime = 0;
  }

  //reading out dimming
  dimPinValue = analogRead(dimPin);
  if (abs(prevDimPin - dimPinValue) > 50) {
    dimFactor = (float)dimPinValue / 1024;
    prevDimPin = dimPinValue;
    if (dimFactor <= 0.1) {
      dimFactor = 0.3;
    } 
    Serial.print("dimfactor: ");
    Serial.println(dimFactor);
    brightness = round(254 * dimFactor);
    intensity = round(dimFactor * 15);
    dimFactorInt = round(255 * dimFactor);
    ledsChanged = true;
    //FastLED.setBrightness(dimFactorInt);
  }

  //displaying speed on round bar  graph
  int barleds = round(kph / 10) + 1;
  if (barleds != prevBarledsOn) {
    ledsChanged = true;
  }

  //Serial.print("Barleds: ");
  //Serial.println(barleds);


  if (ledsChanged) {
    //Serial.println(dimFactorInt);
    FastLED.clear();
    leds[21].setHSV(0, 0, (byte)brightness);
    leds[22].setHSV(0, 0, (byte)brightness);
    for (int i = 0; i <= barleds; i++) {
      leds[21 - i].setHSV(0, 0, (byte)brightness);
    }
    FastLED.show();
    ledsChanged = false;
  }
  prevBarledsOn = barleds;

  //displaying kmstand
  if (!menuactivated) {
    itoa(kph, kphString, 10);
    ltoa(kmstand, kmstandString, 10);
    display_all(kphString, kmstandString, intensity);
  }
  last_timer = timer;

  //check if kilometre was driven and update
  if (pulsecounter >= pulsesPerKm && pulsesPerKm != 0) {
    kmstand++;
    pulsecounter = pulsecounter - pulsesPerKm;
    fram.write32(kmstandAddress, kmstand);
    fram.write32(kmstandAddressBackup, kmstand);
  }
  //delay(1);
  /*leds[0] = CRGB::Red;
    FastLED.show();
    delay(200);
    leds[0] = CRGB::Black;
    FastLED.show();*/
}

//detects and counts incoming pulses from transmission
void pulsedetector() {
  pulseTimerTemp = millis();
  if (pulseTimer < pulseTimerTemp) {
    pulseTime = pulseTimerTemp - pulseTimer;
    pulseTimer = pulseTimerTemp;
    //Serial.println("Pulse detected");
  }

}
