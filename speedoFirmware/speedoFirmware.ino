
//libraries
#include "FRAM.h"
#include "LedController.hpp"
#include "string.h"
#include "Arduino.h"
#include <cstring>
#include <driver/ledc.h>
#include "menuBuilder.h"

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
//Pin definitions
const static uint8_t rowPin[] = { 17, 16, 4 };
const static uint8_t columnPin[] = { 12, 14, 32, 33, 25, 26, 27 };
const static uint8_t vssPin = 34;
const static uint8_t dimPin = 35;
const static uint8_t btnPin = 15;
const static uint8_t kphLedPin = 13;

#define DIN 23
#define CS 5
#define CLK 18
#define Segments 2
//the uninitilized controller object
LedController<Segments, 1> lc;

FRAM fram;
uint32_t start;
uint32_t stop;

TaskHandle_t Task1, Task2;

//don't use the next 10 adresses after these
const static uint8_t kmstandAddress = 0x60;
const static uint8_t kmstandBackupAddress = 0x70;
const static uint8_t pulsesAddress = 0x80;
const static uint8_t pulsesBackupAddress = 0x90;
const static uint8_t pulsesPerKmAddress = 0x40;
const static uint8_t pulsesPerKmBackupAddress = 0x50;

char kphString[3] = { 0 };
char password[] = "password";

uint32_t kmstandBackup = 0;
uint32_t kmstand = 100;
uint32_t trip1 = 4312;
uint32_t trip2 = 8964;
uint32_t taskDelay = 500000;

int pulsecounter = 0;
uint32_t pulsesPerKm = 0;
uint8_t brightness = 8;
int kph = 0;
int lastKph = 0;
int leds = 0;
int columns = 0;
int rows = 0;
int delaytime = 3;
int mainTaskDelay = 500;
int submenu = 0;
int shortpress = 0;
int longpress = 0;

uint64_t pulseTime[3];
uint64_t triggerTime = 0;

bool debouncerActive = false;
bool pwOk = false;

extern MENU mainMenu;
extern MENU pulseMenu;

//prototypes
void updateDisplay();
bool resetTrip(int *);
bool activateUI(int *);
bool setPulsesPerKm(int *);
void signalCounter(int *);
void btnTrigger();

int pV1 = 1;
int pV2 = 2;

MENUITEM mainMenuItems[] = {
  { kphString, &kmstand, NULL, NULL, NULL },
  { "A", &trip1, resetTrip, &pV1, NULL },
  { "b", &trip2, resetTrip, &pV2, NULL },
  { "cAL", &pulsesPerKm, setPulsesPerKm, NULL, NULL },
  { "0FF", NULL, activateUI, NULL, NULL },
};
MENU mainMenu = {
  "MAIN MENU", mainMenuItems, 4, 0, NULL
  // {"Set Temperature", NULL, NULL, tempMenu}
};

MENU *currentMenu = &mainMenu;           // currently selected menu
bool (*currentAction)(int *ptr) = NULL;  // function to execute
void *currentParam = NULL;               // parameter for function to execute

void getBrightness() {
  brightness = analogRead(dimPin);
  if (brightness < 6) {
    brightness = 6;
  }
}

void updateDisplay() {
  lc.clearSegment(0);
  lc.clearSegment(1);
  getBrightness();
  lc.setIntensity(brightness);
  //set 3 digit display
  int size = strlen(currentMenu->items[currentMenu->selected].segment1);
  for (int i = 0; i < size; i++) {
    lc.setChar(0, 2 - i, currentMenu->items[currentMenu->selected].segment1[size - 1], false);
  }
  char value[6];
  utoa(*currentMenu->items[currentMenu->selected].segment2, value, 10);
  size = strlen(value);
  for (int i = 0; i < size; i++) {
    lc.setChar(1, 5 - i, value[size - 1 - i], false);
  }
}

bool resetTrip(int *trip) {
  if (*trip == 1) {
    trip1 = 0;
    updateDisplay();
  } else {
    trip2 = 0;
    updateDisplay();
  }
  return true;
}

bool activateUI(int *) {
  activateBLE();
  return true;
}

bool setPulsesPerKm(int *) {
  longpress = 0;
  int newValue = 0;
  while (1) {
    if (pulsecounter != newValue) {
      newValue = pulsecounter;
      lc.clearSegment(1);
      char value[6];
      utoa(pulsecounter, value, 10);
      int size = strlen(value);
      for (int i = 0; i < size; i++) {
        lc.setChar(1, 5 - i, value[size - 1 - i], false);
      }
    }
    if (longpress) {
      pulsesPerKm = pulsecounter;
      pulsecounter = 0;
      break;
    }
  }
  return true;
}

void signalCounter() {
  pulsecounter++;
  for (int i = 0; i < 2; i++) {
    pulseTime[i] = pulseTime[i + 1];
  }
  pulseTime[2] = esp_timer_get_time() - pulseTime[1];
}

void btnTrigger() {
  if (esp_timer_get_time() - triggerTime > 50000) {
    debouncerActive = false;
  }
  if (!debouncerActive) {
    if (!digitalRead(btnPin)) {
      triggerTime = esp_timer_get_time();
    } else {
      if (esp_timer_get_time() - triggerTime > 500000) {
        longpress++;
      } else {
        shortpress++;
      }
    }
    debouncerActive = true;
  }
}
