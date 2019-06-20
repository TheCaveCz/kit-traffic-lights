/*
   Traffic Lights kit firmware

   The Cave, 2019
   https://thecave.cz

   Licensed under MIT License (see LICENSE file for details)

   Board: Arduino Nano
   Processor: ATmega328p

   Using TaskScheduler and OneButton libraries

*/
#include <OneButton.h>
#include <TaskScheduler.h>

#define PIN_BTN1 A1
#define PIN_BTN2 A2
#define PIN_BTN3 2

#define PIN_CAR_RED1 10
#define PIN_CAR_YELLOW1 12
#define PIN_CAR_GREEN1 11

#define PIN_CAR_RED2 6
#define PIN_CAR_YELLOW2 5
#define PIN_CAR_GREEN2 7

#define PIN_WALK_RED1 3
#define PIN_WALK_GREEN1 4

#define PIN_WALK_RED2 9
#define PIN_WALK_GREEN2 8


#define SIG_CAR_RED 0b00100
#define SIG_CAR_YELLOW 0b00010
#define SIG_CAR_GREEN 0b00001
#define SIG_WALK_RED 0b10000
#define SIG_WALK_GREEN 0b01000

#define SIG_SPEEDUP 0b00100000
#define SIG_TURNOFF 0b01000000
#define SIG_WALK    0b10000000


void keep_me_here() {}

Scheduler scheduler;
Task signalTask(1000, TASK_FOREVER, &signalWorkCb, &scheduler,  true);
uint8_t signalPos;
uint8_t signalRepeats;
uint8_t signalCurrent;
bool signalTurnOffFlag;
OneButton buttonWalk(PIN_BTN1, true);
OneButton buttonTurnOff(PIN_BTN3, true);


const uint8_t signalProgram[] PROGMEM = {
  SIG_WALK_RED | SIG_CAR_YELLOW | SIG_TURNOFF, 2,
  SIG_WALK_RED | SIG_CAR_RED, 2,
  SIG_WALK_GREEN | SIG_CAR_RED | SIG_SPEEDUP, 5 ,
  SIG_WALK_RED | SIG_CAR_RED, 2,
  SIG_WALK_RED | SIG_CAR_RED | SIG_CAR_YELLOW | SIG_TURNOFF, 2,
  SIG_WALK_RED | SIG_CAR_GREEN | SIG_SPEEDUP | SIG_WALK, 20,

  0, 0
};


void signalSet(const uint8_t s) {
  digitalWrite(PIN_WALK_RED1, s & SIG_WALK_RED);
  digitalWrite(PIN_WALK_RED2, s & SIG_WALK_RED);

  digitalWrite(PIN_WALK_GREEN1, s & SIG_WALK_GREEN);
  digitalWrite(PIN_WALK_GREEN2, s & SIG_WALK_GREEN);

  digitalWrite(PIN_CAR_RED1, s & SIG_CAR_RED);
  digitalWrite(PIN_CAR_RED2, s & SIG_CAR_RED);

  digitalWrite(PIN_CAR_YELLOW1, s & SIG_CAR_YELLOW);
  digitalWrite(PIN_CAR_YELLOW2, s & SIG_CAR_YELLOW);

  digitalWrite(PIN_CAR_GREEN1, s & SIG_CAR_GREEN);
  digitalWrite(PIN_CAR_GREEN2, s & SIG_CAR_GREEN);
}

void signalAdvance() {
  if (signalTurnOffFlag && (signalCurrent & SIG_TURNOFF)) {
    signalTurnOffFlag = false;
    signalCurrent = 0;
    signalTask.setCallback(signalBlinkCb);
    signalTask.enable();
    return;
  }
  signalPos++;

  signalCurrent = pgm_read_byte(signalProgram + signalPos * 2);
  signalRepeats = pgm_read_byte(signalProgram + signalPos * 2 + 1);

  signalSet(signalCurrent);
  if (signalRepeats == 0) {
    signalPos = 0xff;
    signalAdvance();
  }
}

void signalWorkCb() {
  if (signalRepeats == 0) {
    signalAdvance();
  }
  signalRepeats--;
}

void signalBlinkCb() {
  signalSet(signalTask.getRunCounter() & 1 ? 0 : SIG_CAR_YELLOW);
  if (signalTurnOffFlag) {
    signalTurnOffFlag = false;
    signalRepeats = 0;
    signalPos = 0xff;
    signalTask.setCallback(signalWorkCb);
    signalTask.enable();
  }
}

void buttonWalkClick() {
  if (signalCurrent & SIG_WALK) {
    signalAdvance();
  }
}

void buttonTurnOffClick() {
  signalTurnOffFlag = true;
  if (signalCurrent & SIG_SPEEDUP) {
    signalAdvance();
  }
}

void setup() {
  signalPos = 0;

  pinMode(PIN_WALK_RED1, OUTPUT);
  pinMode(PIN_WALK_RED2, OUTPUT);
  pinMode(PIN_WALK_GREEN1, OUTPUT);
  pinMode(PIN_WALK_GREEN2, OUTPUT);
  pinMode(PIN_CAR_RED1, OUTPUT);
  pinMode(PIN_CAR_RED2, OUTPUT);
  pinMode(PIN_CAR_YELLOW1, OUTPUT);
  pinMode(PIN_CAR_YELLOW2, OUTPUT);
  pinMode(PIN_CAR_GREEN1, OUTPUT);
  pinMode(PIN_CAR_GREEN2, OUTPUT);

  buttonWalk.attachClick(buttonWalkClick);
  buttonTurnOff.attachClick(buttonTurnOffClick);

  signalPos = 0xff;
  signalAdvance();
}

void loop() {
  buttonWalk.tick();
  buttonTurnOff.tick();
  scheduler.execute();
}
