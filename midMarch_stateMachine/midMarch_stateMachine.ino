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
byte offButtonState = HIGH;
byte flickerState = HIGH;
byte pulseState = HIGH;

int switchReadings[3];
int flickerCount = 0;

// intervals:
long flickerInterval = 500;  // debounce
long flickerInterval_big = int(random(5000, 10000));  // this times in the other flickers gradually

const int buttonInterval = 300; // number of millisecs between button readings


// each's millis tracker:
unsigned long startFlickerMillis = 0; //this keeps track of when the last flickerer was flicked for brightness + triggering steady
unsigned long previousOffMillis;
unsigned long internalFlickerMillis;

// others:
uint32_t currentMax;
int torch;  // which part of the program are we at?
int whichOne = 0;

//light array series:
byte randLights[] = {3, 6, 2, 4, 1, 7, 0, 5, 8};
byte lightSpiral[] = {4, 3, 5, 6, 2, 1, 7, 8, 0 };
byte flickerOrder[] = {0, 3, 6, 5, 6, 7, 4, 2, 8, 6};

// delay lengths:
int delayPts[] = {50, 100, 15, 200, 50};
int delayPts2[] = {10, 50, 8, 100, 25};
int flickerDelay[] = {100, 2000, 123, 223, 825, 400, 80};

int len = 10; // originally were 40 in length
int flickerDelay1[10];  // originally were 40 in length; trying to thread 2 2gether more closely
int flickerDelay2[10];
int flickerDelay3[10];
int flickerDelay4[10];

int flickerDelayA[10];
int flickerDelayB[10];
int flickerDelayC[10];
int flickerDelayD[10];

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

  //------- SEED RANDO LENGTHS to be on and off
  for (int i = 0; i < len; i++) {
    flickerDelay1[i] = int(random(0, 80));  // how long to be on
    flickerDelay2[i] = int(random(0, 50));
    flickerDelay3[i] = int(random(0, 100));
    flickerDelay4[i] = int(random(0, 60));
  }
  for (int i = 0; i < len; i++) {
    flickerDelayA[i] = int(random(0, 150));  // how long to be off for some of them
    flickerDelayB[i] = int(random(0, 150));
    flickerDelayC[i] = int(random(0, 150));
    flickerDelayD[i] = int(random(0, 150));
  }


  // ----- Turn everything off:
  // start off
  for (int i = 0; i < numLights; i ++) {
    tlc.setPWM(i, 0);
  }
  tlc.write();
  torch = 1;
  // hold off for a few seconds:
  delay(5000);

}


void loop() {

  readButton(); // are we trying to turn off? or flicker?

  // if we are not off:
  if (offButtonState != LOW) {

    // set the lights to level:
    // 1. All lights on medium level
    allLevel(torch);

    // after a certain time, begin to flicker:  Triggered by switch
    if (flickerState == LOW)   {
      flickerDance();
    }

    // after all flicking + some time, turn brighter:

    // after flicking brighter + some time, then turn the steady ones off + flickers gradually off

    // rise to pulse

    // then some more flicker

  }// we are not off inside above

} // loop


// this flickers a set of lights gradually over time
void flickerDance() {

  int l = 0;
  // seed new rando's
  for (int i = 0; i < len; i++) {
    flickerDelay1[i] = int(random(0, 80));  // how long to be on
    flickerDelay2[i] = int(random(0, 50));
    flickerDelay3[i] = int(random(0, 100));
    flickerDelay4[i] = int(random(0, 60));
  }
  for (int i = 0; i < len; i++) {
    flickerDelayA[i] = int(random(0, 150));  // how long to be off for some of them
    flickerDelayB[i] = int(random(0, 150));
    flickerDelayC[i] = int(random(0, 150));
    flickerDelayD[i] = int(random(0, 150));
  }

  //------ NEED TO FINESSE THE PAUSES BWTN ROUNDS
  flickerInterval_big = int(random(5000, 10000));
  // flicker a few bulbs gradually over time
  // while the switch is throw and it's just thrown:
  while ( (millis - startFlickerMillis < flickerInterval_big) && (l < 1) ) {
    flicker(l);
    Serial.print("First flicker, less than interval under  "); Serial.println(l);
  }
  //flickerInterval_big = int(random(5000, 10000));
  //after we've triggered our first flickerer and past a time period, go to next light
  while ( (millis - startFlickerMillis > flickerInterval_big) && (l < 4) ) {
    l++;  //increment the light
    Serial.print("past 1 flicker, less than interval under  "); Serial.println(l);
    internalFlickerMillis = millis();
    Serial.print("new internalFlickerMillis and l is: "); Serial.println(l);
    while (millis() - internalFlickerMillis < flickerInterval_big) {
      flicker(l);  // flicker the next light
      Serial.println("internalFlickerMillis is less than big ");
    } // while flicker
    Serial.println("should increment l ");
  } // while we are still under 4

  if ( l >= 4) l = 0;


  // after all flicking + some time, turn brighter:
  // do it manually: read switch to turn brighter

} // flicker state



// this lights up one light in flicker pattern
void flicker(int light) {

  for (char i = 0; i < sizeof(flickerDelay1) / sizeof(int); i++) {
    tlc.setPWM(flickerOrder[light], miniMaxx);  //level
    tlc.write();
    delay(flickerDelay1[i]);
    tlc.setPWM(flickerOrder[light], 0);
    tlc.write();
    // delay off:
    if (i % 3 == 0) {  // every so often, have a larger delay
      delay(flickerDelayA[i]);
    }
    else {
      delay(5);
    }
  } // for-loop for one light



  tlc.setPWM(flickerOrder[light], miniMaxx);  //level

  //********** fun glitches with the above. uncomment one by one:
  //flickerTwo(); // ends dark

  //flickerTravel(light);  // takes them out to darkness individually

  //******

} //flicker

void flickerTwo() {

  for (int i = 0; i < sizeof(flickerDelay1) / sizeof(int); i++) {
    if ( i < 2 ) {
      for (int j = 0; j <= i; j++) {
        tlc.setPWM(flickerOrder[i], miniMaxx);  //level
      }
    }
    tlc.write();
    delay(flickerDelay2[i]);
    if ( i < 2 ) {
      for (int j = 0; j <= i; j++) {
        tlc.setPWM(flickerOrder[i], 0);
      }
    }
    tlc.write();
    // delay off:
    if (i % 3 == 0) {  // every so often, have a larger delay
      delay(flickerDelayA[i]);
    }
    else {
      delay(5);
    }
    // to have lights end on:  commet out line below for some dark spots
    //tlc.setPWM(flickerOrder[i], miniMaxx);  //level
  } // for-loop for one light

}


void flickerTravel(int light) {

  for (int i = 0; i < sizeof(flickerDelay1) / sizeof(int); i++) {
    tlc.setPWM(flickerOrder[light], miniMaxx);  //level
    tlc.write();
    delay(flickerDelay2[i]);
    tlc.setPWM(flickerOrder[light], 0);
    tlc.write();
    // delay off:
    if (i % 3 == 0) {  // every so often, have a larger delay
      delay(flickerDelayA[i]);
    }
    else {
      delay(5);
    }
    // to have lights end on:  uncomment line below for some dark spots
    //tlc.setPWM(flickerOrder[i], miniMaxx);  //level
  } // for-loop for one light
  // /*
  if (light <= numLights) {
    light++;
    flickerTravel(light);
  }
  // */
}

void allLevel(int t) {

  if (t == 1) {
    // 1. All lights on medium level
    for (int i = 0; i < numLights; i ++) {
      tlc.setPWM(i, miniMaxx);
    }
    tlc.write();
  }

  else if (t == 2) {
    // turn on at a higher level:

  }

}

void checkOff() {

  // turn everything off if we have triggered the off switch:
  if (offButtonState == LOW) {
    for (int i = 0; i < numLights; i ++) {
      tlc.setPWM(i, 0);
    }
    tlc.write();
  }
}

void readButton() {
  //  this avoids multiple flashes if the button bounces
  //if (millis() - previousOffMillis >= buttonInterval) {
  //Serial.print("offButtonState is: ");
  //Serial.println(offButtonState);

  if (digitalRead(offButton) == LOW) {
    offButtonState = LOW;
    checkOff();
    //offButtonState = ! offButtonState; //flip flop state
    previousOffMillis += buttonInterval;
  }
  else if (digitalRead(offButton) == HIGH) {
    offButtonState = HIGH;
  }
  //}


  //if (millis() - startFlickerMillis >= flickerInterval) {
  if (digitalRead(flickerButton) == LOW) {
    flickerState = LOW;
    flickerCount = 1;
    //flickerState = ! flickerState; //flip flop state
    //startFlickerMillis += flickerInterval;
    startFlickerMillis = millis(); // take a time stamp for when we start the timer
  }
  //}

  if (digitalRead(flickerButton) == HIGH) {
    flickerState = HIGH;
  }



}

