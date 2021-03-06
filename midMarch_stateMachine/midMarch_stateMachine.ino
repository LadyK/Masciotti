// state machine with time to control

// ----------- LIBRARIES

#include "Adafruit_TLC59711.h"
#include <SPI.h>

// How many boards do you have chained?
#define NUM_TLC59711 1



// ----------- CONSTANTS
// ---------------Pins:
const int offButton = 2;
const int flickerButton = 3;
const int flickerBrighterButton = 4;
const int pulseButton = 5;
//const int steadyOffButton = 6;
const int flicker4Button = 6;
const int steadyOnButton = 10;
const int flicker4OffButton = 7;
const int flicker4OffStateBackButton = 8;
const int burnoutButton = 9;

const int numButtons = 9;
const int potPin = A0;
const int dataPin = 11;
const int clockPin = 13;
const int pwm_pot = A0;
const int flickerDelayPot = A5;

//Number of lights we are controlling:
const int numLights = 9;

// light levels:
const uint32_t maxx = 65535;
const uint32_t miniMaxx = 10000; //21845; //32700;
const uint32_t level_increment = maxx / numLights;  // divide the num of lights by the max, to evenly di


const int switches[] = {offButton, flickerButton, flickerBrighterButton, pulseButton, flicker4Button, steadyOnButton, flicker4OffButton, flicker4OffStateBackButton, burnoutButton};
int switchReadings[9];

Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, clockPin, dataPin);
// ----------- VARIABLES (will change)

// button states:
byte offButtonState = HIGH;
byte flickerState = HIGH;
byte pulseState = HIGH;
byte flickerBrighterState = HIGH;
byte steadyOffState = HIGH;
byte previousSteadyOffState = HIGH;
byte steadyOnState = HIGH;

byte previousFlickerDeathState = LOW;
byte flicker4State = HIGH;
byte flicker4OffState = HIGH;
byte flicker4OffStateBack = HIGH;
byte previousFlicker4state = 0;
byte burnoutButtonstate = HIGH;

bool pwmStart = 0;
bool steady4Flick = false;
bool previousburnoutHappen = false;
bool burnoutHappen = false;

int flickerCount = 0;

// intervals:
long flickerInterval = 500;  // debounce
long flickerInterval_big = int(random(5000, 10000));  // this times in the other flickers gradually
long flickerInterval_b4Bright = 50000;
const int buttonInterval = 300; // number of millisecs between button readings
long steadyOffInterval = 10000;

unsigned long flickerInterval_quiet = 0;

// each's millis tracker:
unsigned long startFlickerMillis = 0; //this keeps track of when the last flickerer was flicked for brightness + triggering steady
unsigned long previousOffMillis;
unsigned long internalFlickerMillis;
unsigned long steadyOffMillis;
unsigned long flickerDeathLimit;
unsigned long flickerBurnOutLimit;


// others:
uint32_t currentMax;
int torch;  // which part of the program are we at?
int whichOne = 0;


//light array series:
byte randLights[] = {3, 6, 2, 4, 1, 7, 0, 5, 8};
byte lightSpiral[] = {4, 3, 5, 6, 2, 1, 7, 8, 0 };

// state of the lights:
char stateOfLight[9];
char lightState; //f, s, d

//****** double check light choices in space:
byte flickerOrder[] = {0, 3, 4, 7, 6, 5, 1, 2, 8};
byte steady[] = {1, 6}; // tack on more

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
//int l_ = 0;

void setup() {
  Serial.begin(9600);
  tlc.begin();
  tlc.write();

  //------ SETUP of SWITCHES:
  for (int i = 0; i < numButtons; i++) {
    pinMode(switches[i], INPUT_PULLUP);
  }

  //------- SEED RANDO LENGTHS to be on and off
  for (int i = 0; i < len; i++) {
    flickerDelay1[i] = int(random(0, 80));  // how long to be on
    flickerDelay2[i] = int(random(0, 20));
    flickerDelay3[i] = int(random(0, 10));
    flickerDelay4[i] = int(random(0, 60));
  }
  for (int i = 0; i < len; i++) {
    flickerDelayA[i] = int(random(0, 150));  // how long to be off for some of them
    flickerDelayB[i] = int(random(0, 70));
    flickerDelayC[i] = int(random(0, 50));
    flickerDelayD[i] = int(random(0, 150));
  }


  // ----- Turn everything off:
  // start off
  for (int i = 0; i < numLights; i ++) {
    tlc.setPWM(i, 0);
    stateOfLight[flickerOrder[i]] = 's';
  }
  tlc.write();
  torch = 1;
  // hold off for a few seconds:
  delay(200);

}


void loop() {


  readButton(); // are we trying to turn off? or flicker? or brighter flicker/


  // if we are not off:
  if (offButtonState != LOW) {

    // set the lights to level:
    // 1. All lights on medium level
    if (steadyOnState == LOW) {
      allLevel(); // torch

      //Serial.print("steadyOnState is: "); Serial.println(steadyOnState);
    }
    /*
        else if (steadyOnState == HIGH) {
          for (int i = 0; i < numLights; i ++) {
            tlc.setPWM(i, 0);
          }
          tlc.write();
    */


    // check All of the switches:

    if (flickerState == LOW)   {
      //flickerDance();
      //flickerDance2();

      // turns between all of them:
      flickerCount = flickerDance3(flickerCount);
      // /*
      for (int i = 0; i < 4; i++) {  // only does for in flickerOrder Array
        //flickerCount = flickerDance4(i);
      }
      //*/
    }



    if ( flicker4State == LOW) {

      //have we already started recently?
      if (steady4Flick != true) {
        /* //gradually bring 4 in
          for (int i = 0; i < 4; i++) {
           flicker4Gradually(i);
           if ( i >= 3)
        */steady4Flick = true;
        // }

      }

      // once they are all in flash them the same:
      while (steady4Flick == true) {
        flicker4(0);
        //readButton(); // not working ... Try again with flick. Maybe just read the next switch in the series?
        if (digitalRead(flicker4OffButton) == LOW) {
          steady4Flick = false;
        }
        //Serial.println(steady4Flick);
      }
    }
    //steady4Flick = false;

    if (flicker4State == HIGH) {
      steady4Flick = false;
    }


    // turn the 4 flicker's off:
    if (flicker4OffState == LOW) {
      flicker4Off();
      previousFlicker4state = 1 ; // invert the state
    }


    // turn the 4 flicker's back ON and at the current level:
    if ((flicker4OffStateBack == LOW) && (previousFlicker4state == 1) ) {
      previousFlicker4state = 0 ; // invert the state
      flicker4OffBack();
    }

    Serial.print("before if-statement: "); Serial.println(burnoutButtonstate);
    //rise, strain then burn out:
    if ( (burnoutButtonstate == LOW) && (burnoutHappen = true) && ( previousburnoutHappen == false ) ) { //&& (burnoutHappen = true)
      //int pot_r = analogRead(pwm_pot);
      //Serial.print("burnout pot is: "); Serial.println(pot_r);
      //uint32_t pace = map(pot_r, 0, 1023, 0, rangeLength);
      // 1, 3, 5, 15, 17, 51, 85, 255, 257, 771, 1285, 3855, 4369, 13107, 21845, 65535
      uint32_t pace = 771; //51;
      Serial.println(pace);
      strainBurnOut(pace);
      flickerBurnOutLimit = millis();
      //burnoutHappen = false;  // we only want to happen once
      previousburnoutHappen = true;
    }

    if (millis() - flickerBurnOutLimit > 20000) {  // after time, come back enable
      previousburnoutHappen = false;
      // Serial.println("reset + re-enable");
    }

    /**** not using this one ***/
    // after all flicking + some time, turn brighter + die
    if (  flickerBrighterState == LOW && previousFlickerDeathState == LOW) {
      flickerDeathLimit = millis();
      Serial.println("begin the death brillance");
      // go through 4 lights
      //******* time delay between each ??
      for (int i = 0; i < 4; i++) {
        // gradually make them burn out:
        //for (uint32_t j = miniMaxx; j < maxx; j += level_increment) {
        flickerDeath(i, 0);

        //}
      }
      previousFlickerDeathState = HIGH;  // so as to not repeat
      //delay(3000); // just to make it more obvious for testing/operating
    }

    // enable the process to be re-start the process
    if (millis() - flickerDeathLimit > 20000) {  // after time, come back enable
      previousFlickerDeathState = LOW;
      // Serial.println("reset + re-enable");
    }
    /**** not using this one ***/


    // then turn the steady ones off

    /*
        if ( steadyOffState == LOW &&  previousSteadyOffState != LOW) {

          //Serial.println("we flip-flopped");
          //*********** delay between each one needed
          //for (int i = 0; i < 2; i++) {
          //torch = 0;
          steadyOff();
          //delay(500);
          //}
          previousSteadyOffState = steadyOffState;


        }
    */
    // rise to pulse
    if (pulseState == LOW) {
      int pot_r = analogRead(pwm_pot);
      Serial.print("pot is: "); Serial.println(pot_r);
      uint32_t pace = map(pot_r, 0, 1023, 0, rangeLength);
      Serial.println(pace);
      if (pwmStart == 0) { // only the first time through
        pwmStart = 1;
        slowRise_all();
      }
      pwm_all(range[pace]);

    }

    // then some more flicker



    //previousSteadyOffState = steadyOffState;
  }// we are on & inside above

} //----------- of main loop !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!


void strainBurnOut(uint32_t pace_) {
  uint32_t last;

  //------ rise to 2/3 light level:
  // 1, 3, 5, 15, 17, 51, 85, 255, 257, 771, 1285, 3855, 4369, 13107, 21845, 65535
  for (uint32_t level_increment = miniMaxx; level_increment <= maxx; level_increment += 1285) {  // for (uint32_t level_increment = 0; level_increment <= 65535; level_increment += 255) { works
    //Serial.println("inside increase");
    for (int i = 0; i < 9; i++) {
      tlc.setPWM(i, level_increment);
    }
    tlc.write(); // time here and all at once w/o delay below;  w/delay below time btwn ea step
    delay(100); // time btwn each stage
    //Serial.println(level_increment);
    last = level_increment;
  }


  // flicker back and forth quickly and in the midField of range
  // do this for a set period of time:
  int timeIncrement = 4000;
  unsigned long initial = millis();
  if (millis() - initial < timeIncrement) {
    //fast flicker

    for (char i = 0; i < sizeof(flickerDelay1) / sizeof(int); i++) {
      for (int j = 0; j <  numLights; j++) {
        if (j % 2) tlc.setPWM(j, maxx);
        else tlc.setPWM(j, miniMaxx);  //level
        //tlc.write();
      }
      tlc.write();
      delay(flickerDelay2[i]);
      for (int j = 0; j <  numLights; j++) {
        tlc.setPWM(j, 0);
        //tlc.write();
      }
      tlc.write();

      // delay off:
      if (i % 3 == 0) {  // every so often, have a larger delay
        delay(flickerDelayB[i]);
      }
      else {
        delay(5);
      }
      // leave on when exiting:
      for (int j = 0; j <  numLights; j++) {
        tlc.setPWM(j, 21845 );  // 2/3 level  21845  43690
      }
      tlc.write();

    } // for-one sequence
  }// time increment





  // then rise to full brillance:
  for (uint32_t level_increment = 21845; level_increment <= maxx; level_increment *= 5) {  // for (uint32_t level_increment = 0; level_increment <= 65535; level_increment += 255) { works
    //Serial.println("inside increase");
    for (int i = 0; i < 9; i++) {
      tlc.setPWM(i, level_increment);
    }
    tlc.write(); // time here and all at once w/o delay below;  w/delay below time btwn ea step
    delay(100);
  }

  //flicker back and forth quickly  (on/off super fast)
  // for a short period of time

  int fastFlickerDur = 3000;
  unsigned long tstamp = millis();
  if (millis() - tstamp < fastFlickerDur) {
    //fast flicker

    for (char i = 0; i < sizeof(flickerDelay1) / sizeof(int); i++) {
      for (int j = 0; j <  numLights; j++) {
        if (j % 2) tlc.setPWM(j, maxx);
        else tlc.setPWM(j, miniMaxx);  //level
        //tlc.write();
      }

      tlc.write();
      delay(flickerDelay3[i]);
      for (int j = 0; j <  numLights; j++) {
        tlc.setPWM(j, 0);
        //tlc.write();
      }
      tlc.write();

      // delay off:
      if (i % 3 == 0) {  // every so often, have a larger delay
        delay(flickerDelayC[i]);
      }
      else {
        delay(5);
      }

    }// for-one sequence
  }  // time increment




  // all off

  for (int i = 0; i < numLights; i ++) {
    tlc.setPWM(i, 0);
  }
  tlc.write();


}

// goes across first 4 in flickerOrder to sync with 4 flickers at once
int flickerDance4(int lite) {
  // flick the light for a bit before moving to next one
  // keeps rotating between 4
  //int lite = 0;
  // pick interval to be quiet
  int rando = random(0, 10);
  if (rando > 6) {
    //****** CHANGE QUIET LENGTHS HERE:
    flickerInterval_quiet = int(random(1000, 5000));  // pick a delay interval before nxt light
    Serial.println("shorter quiet period");
  }
  else {
    //****** CHANGE QUIET LENGTHS HERE:
    flickerInterval_quiet = int(random(5000, 8000));
    Serial.println("longer quiet period");
  }
  //not to delay long with the first light
  if ( lite == 0) {
    flickerInterval_big = 0;
    flickerInterval_quiet = 2000;
  }
  else {
    flickerInterval_big = int(random(5000, 10000)); // flicker duration
    startFlickerMillis = millis(); // initial time stamp
  }
  // while we are less than both the pause and the flicker interval:

  while (millis() - startFlickerMillis < (flickerInterval_quiet + flickerInterval_big ) ) {

    // keep flickering our light
    while (millis() - startFlickerMillis < flickerInterval_big) {
      flicker(flickerOrder[lite]);
    } //while

    //Serial.println("done flickering");
    //Serial.println("still waiting for quiet to be done");
  }// while

  return lite;
}

int flickerDance3(int l_) {
  // flick the light for a bit before moving to next one
  // keeps rotating

  // pick interval to be quiet
  int rando = random(0, 10);
  if (rando > 6) {
    //****** CHANGE QUIET LENGTHS HERE:
    flickerInterval_quiet = int(random(1000, 5000));  // pick a delay interval before nxt light
    Serial.println("shorter quiet period");
  }
  else {
    //****** CHANGE QUIET LENGTHS HERE:
    flickerInterval_quiet = int(random(5000, 10000));
    Serial.println("longer quiet period");
  }

  flickerInterval_big = int(random(5000, 10000)); // flicker duration
  startFlickerMillis = millis(); // initial time stamp
  while (millis() - startFlickerMillis < (flickerInterval_quiet + flickerInterval_big ) ) {
    // keep flickering our light

    while (millis() - startFlickerMillis < flickerInterval_big) {
      flicker(l_);
    } //while


  }// while


  l_++;
  return l_;

}

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
  int rando = random(0, 10);
  if (rando > 7) {
    flickerInterval_big = int(random(5000, 10000)); // pick a delay interval before nxt light
  }
  else {
    flickerInterval_big = int(random(15000, 35000)); // pick a delay interval before nxt light
  }
  //Serial.println(rando);

  while (( millis() - startFlickerMillis < flickerInterval_big) && (millis() - startFlickerMillis < flickerInterval_b4Bright) ) {
    Serial.println(l);
    flicker(l);
    stateOfLight[l] = 'f';
    //readButton(); // uncomment this to not have it move. bade idea
  }

  // move on to flicker a few bulbs gradually over time
  // so after we've triggered our first flickerer and past a time period, go to next light
  // first period before getting brighter
  while ( (millis - startFlickerMillis > flickerInterval_big) && (millis() - startFlickerMillis < flickerInterval_b4Bright) ) { //( l < 4) //millis() - startFlickerMillis < flickerInterval_b4Bright
    l++;  //increment the light
    stateOfLight[l] = 'f';
    //Serial.println(l);
    internalFlickerMillis = millis();
    Serial.println(l);
    while (millis() - internalFlickerMillis < flickerInterval_big) {
      flicker(l);  // flicker the next light
    } // while flicker
  } // next while up
} // flicker Dance


// working several simultaneously
void flicker4(int numL) {

  for (char i = 0; i < sizeof(flickerDelay1) / sizeof(int); i++) {
    //for (int j = numL; j < (numL + 4); j++) {
    for (int j = 0; j <  4; j++) {
      tlc.setPWM(flickerOrder[j], miniMaxx);  //level
      //tlc.write();
    }
    tlc.write();
    delay(flickerDelay1[i]);
    //for (int j = numL; j < (numL + 4); j++) {
    for (int j = 0; j <  4; j++) {
      tlc.setPWM(flickerOrder[j], 0);
      //tlc.write();
    }
    tlc.write();

    // delay off:
    if (i % 3 == 0) {  // every so often, have a larger delay
      delay(flickerDelayA[i]);
    }
    else {
      delay(5);
    }
    // leave on when exiting:
    //for (int j = numL; j < (numL + 4); j++) {
    for (int j = 0; j <  4; j++) {
      tlc.setPWM(flickerOrder[j], miniMaxx);  //level
      // stateOfLight[flickerOrder[j]] = 'f';  // or steady

      tlc.write();
      //flickerTravel(j);
    }


  } // for-loop for one light



  //********** fun glitches with the above. uncomment one by one:
  //flickerTwo(); // ends dark

  //flickerTravel(light);  // takes them out to darkness individually

  //******

} // flicker4

// working several simultaneously
void flicker4Gradually(int numL) {

  for (char i = 0; i < sizeof(flickerDelay1) / sizeof(int); i++) {
    for (int j = 0; j < numL; j++) {
      //for (int j = 0; j <  4; j++) {
      tlc.setPWM(flickerOrder[j], miniMaxx);  //level
      //tlc.write();
    }
    tlc.write();
    delay(flickerDelay1[i]);
    for (int j = 0; j < numL; j++) {
      //for (int j = 0; j <  4; j++) {
      tlc.setPWM(flickerOrder[j], 0);
      //tlc.write();
    }
    tlc.write();

    // delay off:
    if (i % 3 == 0) {  // every so often, have a larger delay
      delay(flickerDelayA[i]);
    }
    else {
      delay(5);
    }
    // leave on when exiting:
    for (int j = 0; j < numL; j++) {
      //for (int j = 0; j <  4; j++) {
      tlc.setPWM(flickerOrder[j], miniMaxx);  //level
      // stateOfLight[flickerOrder[j]] = 'f';  // or steady

      // tlc.write();  //**** interesting glitch here if this is the only internal write uncommented
      //flickerTravel(j);
    }
    tlc.write();


  } // for-loop for one light



  //********** fun glitches with the above. uncomment one by one:
  //flickerTwo(); // ends dark

  //flickerTravel(light);  // takes them out to darkness individually

  //******

} // flicker4

void flicker4Off() {
  for (int j = 0; j <  4; j++) {
    tlc.setPWM(flickerOrder[j], 0);  //level
    tlc.write();
  }
}

void flicker4OffBack() {
  for (int j = 0; j <  4; j++) {
    tlc.setPWM(flickerOrder[j], miniMaxx);  //level
    tlc.write();
  }
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

  //stateOfLight[flickerOrder[light]] = 'f';
  int numL = flickerOrder[light];
  stateOfLight[numL] = 'f';


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

  //Serial.print("Level is: "); Serial.println(maxx);
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
  stateOfLight[flickerOrder[light]] = 'd';
  //int numL = flickerOrder[light];
  //stateOfLight[numL] = 'd';

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
    tlc.setPWM(l1_, 0); // off
    tlc.setPWM(l2_, 0); // off
    tlc.write();
    delay(flickerDelay[i + 1]);
    tlc.setPWM(l1_, miniMaxx);  // randomly pick 2 lights for each call
    tlc.setPWM(l2_, miniMaxx);
  }


}


void steadyOff() {
  // take the constant ones off
  // perhaps the others are flickering
  Serial.println("we are in steady");
  // take a light from the constant array
  for (int i = 0; i < numLights; i++) { //sizeof(steady) / sizeof(int)
    Serial.print("State of Light "); Serial.print(i); Serial.print(" is: "); Serial.println(stateOfLight[i]);
    // if we are a flickering light
    if ((stateOfLight[i] = 'f') || (stateOfLight[i] = 'd')) {
      //continue;
      Serial.println("f or d, moving along");
    } else if (stateOfLight[i] = 's') {

      //turn that light off with a firey exit:
      uint32_t pace_ = level_increment;

      for (uint32_t level_increment_ = miniMaxx; level_increment_ < maxx; level_increment_ += pace_) {
        // have it increase rapidly,
        // close to end? Get brighter.....or waver back and forth quickly?

        if ( level_increment_ >= ( (maxx / 3) * 2 ) ) {
          level_increment_++;
        }
        tlc.setPWM(i, level_increment_);  // turn that direct light...
        tlc.write(); // print to light
        delay(200);  //getting faster as it gets larger??
      }
      Serial.println("we off");
      tlc.setPWM(i, 0); // turn off  // turn that direct light...
      stateOfLight[i] = 'd';
      // Serial.println("next light");
      // Serial.println(steady[i]);
    }

    //Serial.println("f or d, moving along");


  }
  //take a time stamp for starting
  // are we beyond the interval
  /*
    if (millis() - steadyOffMillis > steadyOffInterval) {

    }
  */
  // darkness
  previousSteadyOffState = steadyOffState;

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
    // to have lights end on:  commet out 2 lines below for some dark spots
    //tlc.setPWM(flickerOrder[i], miniMaxx);  //level
    int numL = flickerOrder[i];
    stateOfLight[numL] = 'f';

  } // for-loop for one light

}


void flickerTravel(int light) {
  //stateOfLight[light] = 'f';

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

void allLevel() { //int skip


  // All lights on medium level
  for (int i = 0; i < numLights; i ++) {
    if ( ( stateOfLight[i] == 'f') || ( stateOfLight[i] == 'd') ) {
      // tlc.setPWM(i, 0);
      continue;
    }
    //}
    else {
      tlc.setPWM(i, miniMaxx);
      stateOfLight[i] = 's';
    }
  }
  tlc.write();
  //}

  //else if (t == 2) {
  // turn on at a higher level:

  // }

}


void slowRise_all() {
  uint32_t startLevel = 0;
  uint32_t level_increment = maxx / numLights;  // divide the num of lights by the max, to evenly distribute
  uint32_t currentMax = level_increment + startLevel;  //our limit for each level will gradually increase for each light as we progress
  int timer = 30;
  int c = 0;
  while (c < numLights) { // for every light
    if (startLevel < currentMax * 0.5) {
      tlc.setPWM(c, startLevel);
      tlc.write();
    }

    if (startLevel >= currentMax * 0.5) {
      tlc.setPWM(c + 1, startLevel);
      tlc.write();
    }

    startLevel = startLevel + 100;


    c++; // move to the next light?
  }// while
}

void pwm_all(uint32_t pace_) {
  uint32_t last;
  //uint32_t maxx = 65535;
  //up:
  //uint32_t level_increment;
  // put a pot up to pick one of these values from an array mapped to it's dial
  // 1, 3, 5, 15, 17, 51, 85, 255, 257, 771, 1285, 3855, 4369, 13107, 21845, 65535
  for (uint32_t level_increment = 0; level_increment <= 65535; level_increment += pace_) {  // for (uint32_t level_increment = 0; level_increment <= 65535; level_increment += 255) { works
    //Serial.println("inside increase");
    for (int i = 0; i < 9; i++) {
      tlc.setPWM(i, level_increment);
    }
    tlc.write(); // time here and all at once w/o delay below;  w/delay below time btwn ea step
    //delay(10); // time btwn each stage
    //Serial.println(level_increment);
    last = level_increment;
  }

  //down
  //uint32_t level_incrementb;
  for (uint32_t level_incrementb = last; level_incrementb > 0; level_incrementb -= pace_) {
    //Serial.println("inside decrease");
    for (int i = 0; i < 9; i++) {
      tlc.setPWM(i, level_incrementb);
    }
    tlc.write(); // time here and all at once w/o delay below; w/delay below time btwn ea step
    //delay(10); // time btwn each stage
    //Serial.println(level_incrementb);
  }

}


void checkOff() {

  // turn everything off if we have triggered the off switch:
  if (offButtonState == LOW) {
    for (int i = 0; i < numLights; i ++) {
      tlc.setPWM(i, 0);
      stateOfLight[i] = 's';
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
    //Serial.print("flickerCount is: ");  Serial.println(flickerCount);
    //flickerCount = 1;
    //flickerState = ! flickerState; //flip flop state
    //startFlickerMillis += flickerInterval;
    //startFlickerMillis = millis(); // take a time stamp for when we start the timer
  }
  //}

  else if (digitalRead(flickerButton) == HIGH) {
    flickerState = HIGH;
    steady4Flick = false;
    flickerCount = 0;
  }

  if (digitalRead(flickerBrighterButton) == LOW) {
    flickerBrighterState = LOW;
    //previousFlickerDeathState = LOW;

  }

  else if (digitalRead(flickerBrighterButton) == HIGH) {
    flickerBrighterState = HIGH;
    //previousFlickerDeathState = HIGH;


  }


  if (digitalRead(flicker4Button) == LOW) {

    flicker4State = LOW;


  }
  else if (digitalRead(flicker4Button) == HIGH) {
    flicker4State = HIGH;

  }

  if (digitalRead(steadyOnButton) == LOW) {

    steadyOnState = LOW;
  }

  else if (digitalRead(steadyOnButton) == HIGH) {

    steadyOnState = HIGH;
  }

  if (digitalRead(pulseButton) == LOW) {
    pulseState = LOW;
  }

  else if (digitalRead(pulseButton) == HIGH) {
    pulseState = HIGH;
    pwmStart == 0;
  }

  if (digitalRead(flicker4OffButton) == LOW) {
    flicker4OffState = LOW;
  }
  else if (digitalRead(flicker4OffButton) == HIGH) {
    flicker4OffState = HIGH;
  }

  if (digitalRead(flicker4OffStateBackButton) == LOW) {
    flicker4OffStateBack = LOW;
  }
  else if (digitalRead(flicker4OffStateBackButton) == HIGH) {
    flicker4OffStateBack = HIGH;
  }

  if (digitalRead(burnoutButton) == HIGH) {
    burnoutButtonstate = HIGH;
    //burnoutHappen = false;
  }
  else if (digitalRead(burnoutButton) == LOW) {
    burnoutButtonstate = LOW;
    burnoutHappen = true;
    //previousburnoutHappen = false;
  }



  else {
    // allLevel(torch);

  }

}


