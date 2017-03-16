// state machine with time to control

// ----------- LIBRARIES

#include "Adafruit_TLC59711.h"
#include <SPI.h>

// How many boards do you have chained?
#define NUM_TLC59711 1



// ----------- CONSTANTS
const int offButton = 2;
const int flickerButton = 3;
const int flickerBrighterButton = 4;
const int pulseButton = 5;
//const int flickerButton2 = 5; ??
const int potPin = A0;
const int dataPin = 11;
const int clockPin = 13;

//Number of lights we are controlling:
const int numLights = 9;

// light levels:
const uint32_t maxx = 65535;
const uint32_t miniMaxx = 10000; //21845; //32700;
const uint32_t level_increment = maxx / numLights;  // divide the num of lights by the max, to evenly di


const int switches[] = {offButton, flickerButton, flickerBrighterButton, pulseButton};

Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, clockPin, dataPin);
// ----------- VARIABLES (will change)

// button states:
byte offButtonState = HIGH;
byte flickerState = HIGH;
byte pulseState = HIGH;
byte flickerBrighterState = HIGH;

int switchReadings[4];
int flickerCount = 0;

// intervals:
long flickerInterval = 500;  // debounce
long flickerInterval_big = int(random(5000, 10000));  // this times in the other flickers gradually
long flickerInterval_b4Bright = 50000;
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
byte flickerOrder[] = {0, 3, 4, 7, 6, 5, 1, 2, 8, 6};

// delay lengths:
int delayPts[] = {50, 100, 15, 200, 50};
int delayPts2[] = {10, 50, 8, 100, 25};
//int flickerDelay[] = {100, 2000, 123, 223, 825, 400, 80};
int flickerDelay[] = {3, 20, 83, 33, 5, 40, 10};

int len = 20; // originally were 40 in length
int flickerDelay1[20];  // originally were 40 in length;
int flickerDelay2[20];
int flickerDelay3[20];
int flickerDelay4[20];

int flickerDelayA[20];
int flickerDelayB[20];
int flickerDelayC[20];
int flickerDelayD[20];

// flicker death is longer and brighter
int flickerDelay42[40];
int flickerDelayZ[40];

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

  readButton(); // are we trying to turn off? or flicker? or brighter flicker/

  // if we are not off:
  if (offButtonState != LOW) {

    // set the lights to level:
    // 1. All lights on medium level
    allLevel(torch);

    // after a certain time, begin to flicker:  Triggered by switch
    if (flickerState == LOW)   {
      flickerDance();
      //flickerDance2();
    }

    // after all flicking + some time, turn brighter + die
    if (flickerBrighterState == LOW) {
      // go through 4 lights
      //******* time delay between each ??
      for (int i = 0; i < 4; i++) {
        // gradually make them burn out:
        //for (uint32_t j = miniMaxx; j < maxx; j += level_increment) {
        flickerDeath(i, 0);

        //}
      }
      delay(3000); // just to make it more obvious for testing/operating
    }


    // then turn the steady ones off

    // rise to pulse

    // then some more flicker

  }// we are on & inside above

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


  // flick the light for a bit before moving to next one
  startFlickerMillis = millis(); // initial time stamp
  //********   change random numbers for longer breaks btwn
  flickerInterval_big = int(random(5000, 10000)); // pick a delay interval before nxt light
  while (( millis() - startFlickerMillis < flickerInterval_big) && (millis() - startFlickerMillis < flickerInterval_b4Bright) ) {
    flicker(l);
    //readButton(); // uncomment this to not have it move. bade idea
  }

  // move on to flicker a few bulbs gradually over time
  // so after we've triggered our first flickerer and past a time period, go to next light
  // first period before getting brighter
  while ( (millis - startFlickerMillis > flickerInterval_big) && (millis() - startFlickerMillis < flickerInterval_b4Bright) ) { //( l < 4) //millis() - startFlickerMillis < flickerInterval_b4Bright
    l++;  //increment the light
    //Serial.println(l);
    internalFlickerMillis = millis();
    //Serial.println("new internalFlickerMillis");
    //readButton(); // uncomment this to not have it move. bade idea

    while (millis() - internalFlickerMillis < flickerInterval_big) {
      flicker(l);  // flicker the next light
      //Serial.println("internalFlickerMillis is less than big ");
      //readButton(); // uncomment this to not have it move. bade idea
    } // while flicker

    // ------ every so often, flick a couple random lights:
    int rA = ceil(random(0, numLights));
    int rB = ceil(random(0, numLights));
    // **************glitch
    if ( l < 2) two_glitchFlicker(rA, rB) ;
    //if ( l >= 4) l = 0;
    /*
       How many times for the above process to happen?
       create a variable to count the number of times to come through and limit???
       or reposition  startFlickerMillis = millis(); // initial time stamp ??
    */
  }

  // after all flicking + some time, turn brighter:
  /*
    // do it manually: read switch to turn brighter
    if (millis() - startFlickerMillis > flickerInterval_b4Bright) {
      // see if brighter button is pressed
      readButton();
      Serial.println("waiting for death button");
      /*
          if (flickerBrighterState = LOW) {
            Serial.println("Death after waiting");
            flickerDeath();
          }
  */
  // } // ready to be brighter + die

} // flicker Dance

// working several simultaneously
void flickerDance2() {
  for (char i = 0; i < sizeof(flickerDelay1) / sizeof(int); i++) {
    for (int j = 0; j < 4; j++) {
      tlc.setPWM(flickerOrder[j], miniMaxx);  //level
      tlc.write();
    }
    
    delay(flickerDelay1[i]);
    for (int j = 0; j < 4; j++) {
      tlc.setPWM(flickerOrder[j], 0);
      tlc.write();
    }

    // delay off:
    if (i % 3 == 0) {  // every so often, have a larger delay
      delay(flickerDelayA[i]);
    }
    else {
      delay(5);
    }
    // leave on when exiting:
    for (int j = 0; j < 4; j++) {
      tlc.setPWM(flickerOrder[j], miniMaxx);  //level
      tlc.write();
    }

    //readButton(); // uncomment this to not have it move. bad idea
  } // for-loop for one light




  //********** fun glitches with the above. uncomment one by one:
  //flickerTwo(); // ends dark

  //flickerTravel(light);  // takes them out to darkness individually

  //******

}

// this lights up one light in flicker pattern
void flicker(int light) {
  //Serial.print("Level is: "); Serial.println(miniMaxx);
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
    //readButton(); // uncomment this to not have it move. bade idea
  } // for-loop for one light


  // leave on when exiting:
  tlc.setPWM(flickerOrder[light], miniMaxx);  //level
  tlc.write();
  //********** fun glitches with the above. uncomment one by one:
  //flickerTwo(); // ends dark

  //flickerTravel(light);  // takes them out to darkness individually

  //******

} //flicker

// death flicker pattern
void flickerDeath(int light, bool endState) {

  // Serial.println("We are trying to die");
  // Serial.print("the light is:  "); Serial.println(light);
  // make it faster
  for (int i = 0; i < (len * 2); i++) {
    flickerDelay42[i] = int(random(0, 50));  // how long to be on
    flickerDelayZ[i] = int(random(0, 75));
  }

  Serial.print("Level is: "); Serial.println(maxx);
  //Serial.print("the light is slot:  "); Serial.println(flickerOrder[light]);
  for (int i = 0; i < sizeof(flickerDelay42) / sizeof(int); i++) {
    tlc.setPWM(flickerOrder[light], maxx);  //level
    tlc.write();
    delay(flickerDelay42[i]);
    tlc.setPWM(flickerOrder[light], 0);
    tlc.write();
    // delay off:
    if (i % 3 == 0) {  // every so often, have a larger delay
      delay(flickerDelayZ[i]);
    }
    else {
      delay(5);
    }
  } // for-loop for one light


  // leave off when exiting:
  if (endState == 0) tlc.setPWM(flickerOrder[light], 0); //level



} //flickerdeath


void two_glitchFlicker(int l1_, int l2_) {

  for (int i = 0; i < 7; i++) { // maybe a longer range than 7
    //uint32_t range = map(flickerDelay[i], 0, 255, 0, maxx);
    tlc.setPWM(l1_, maxx);  // randomly pick 2 lights for each call
    tlc.setPWM(l2_, maxx);
    tlc.write();
    delay(flickerDelay[i]);  // want this delay to be sporadically longer
    tlc.setPWM(l1_, 0); // off
    tlc.setPWM(l2_, 0); // off
    tlc.write();
    delay(flickerDelay[i + 1]);
    tlc.setPWM(l1_, maxx);  // randomly pick 2 lights for each call
    tlc.setPWM(l2_, maxx);
  }


}

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
    // previousOffMillis += buttonInterval;
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

  else if (digitalRead(flickerButton) == HIGH) {
    flickerState = HIGH;
  }

  if (digitalRead(flickerBrighterButton) == LOW) {
    flickerBrighterState = LOW;
  }

  else if (digitalRead(flickerBrighterButton) == HIGH) {
    flickerBrighterState = HIGH;
  }

  else {
    allLevel(torch);

  }

}


