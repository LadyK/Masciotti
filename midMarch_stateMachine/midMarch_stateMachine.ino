// state machine with time to control

// ----------- LIBRARIES

#include "Adafruit_TLC59711.h"
#include <SPI.h>

// How many boards do you have chained?
#define NUM_TLC59711 1



// ----------- CONSTANTS
const int offButton = 2;
const int flickerButton = 3;
const int pulseButton = 4;
//const int flickerButton2 = 5; ??
const int potPin = A0;
const int dataPin = 11;
const int clockPin = 13;

//Number of lights we are controlling:
const int numLights = 9;

// light levels:
const uint32_t maxx = 65535;
const uint32_t miniMaxx = 32700;
const uint32_t level_increment = maxx / numLights;  // divide the num of lights by the max, to evenly di


const int switches[] = {offButton, flickerButton, pulseButton};

Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, clockPin, dataPin);
// ----------- VARIABLES (will change)

// button states:
bool offButtonState = HIGH;
bool flickerState = HIGH;
bool pulseState = HIGH;

int switchReadings[3];

// intervals:
long flickerInterval = int(random(5000, 10000));  // this times in the other flickers gradually
long flickerCount;
const int buttonInterval = 300; // number of millisecs between button readings


// each's millis tracker:
unsigned long lastFlickerMillis; //this keeps track of when the last flickerer was flicked for brightness + triggering steady
unsigned long previousOffMillis;


// others:
uint32_t currentMax;


//light array series:
byte randLights[] = {3, 6, 2, 4, 1, 7, 0, 5, 8};
byte lightSpiral[] = {4, 3, 5, 6, 2, 1, 7, 8, 0 };
byte flickerOrder[] = {0, 3, 4, 5, 6, 7, 4, 2, 8, 6};

// delay lengths:
int delayPts[] = {50, 100, 15, 200, 50};
int delayPts2[] = {10, 50, 8, 100, 25};
int flickerDelay[] = {100, 2000, 123, 223, 825, 400, 80};

byte flickerDelay1[40];
byte flickerDelay2[40];
byte flickerDelay3[40];
byte flickerDelay4[40];

byte flickerDelayA[40];
byte flickerDelayB[40];
byte flickerDelayC[40];
byte flickerDelayD[40];

// pwm pace is largely set here:
long range[] = { 255, 257, 771, 1285, 3855, 4369};
int rangeLength = 6;


void setup() {
  Serial.begin(9600);
  tlc.begin();
  tlc.write();

  //------ SETUP of SWITCHES:
  for (int i = 0; i < 4; i++) {
    pinMode(switches[i], INPUT_PULLUP);
  }

  // ----- Turn everything off:
  // start off
  for (int i = 0; i < numLights; i ++) {
    tlc.setPWM(i, 0);
  }
  tlc.write();
  // hold off for a few seconds:
  delay(5000);
}


void loop() {

  readButton(); // are we trying to turn off?

  // 1. All lights on medium level
  for (int i = 0; i < numLights; i ++) {
    tlc.setPWM(i, miniMaxx);
  }
  tlc.write();
  delay(5000);

  // after a certain time, begin to flicker:  Triggered by switch
  // gradually have more flickers come in (OR listen to other switches for more flickers)
  // after all flicking + some time, turn brighter:

  // after flicking brighter + some time, then turn the steady ones off + flickers gradually off

  // rise to pulse

  // then some more flicker


}

void readButton() {
  //  this avoids multiple flashes if the button bounces
  if (millis() - previousOffMillis >= buttonInterval) {
    if (digitalRead(offButton) == LOW) {
      offButtonState = LOW;
      previousOffMillis += buttonInterval;
    }

  }
  else if (digitalRead(offButton) == HIGH) {
    offButtonState = HIGH;
  }

  // turn everything off if we have triggered the off switch:
  if (offButtonState == LOW) {
    for (int i = 0; i < numLights; i ++) {
      tlc.setPWM(i, 0);
    }
    tlc.write();
  }

}

