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

const int numButtons = 8;
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


const int switches[] = {offButton, flickerButton, flickerBrighterButton, pulseButton, flicker4Button, steadyOnButton, flicker4OffButton, flicker4OffStateBackButton};
int switchReadings[8];

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
bool pwmStart = 0;
byte previousFlickerDeathState = LOW;
byte flicker4State = HIGH;
byte flicker4OffState = HIGH;
byte flicker4OffStateBack = HIGH;
byte previousFlicker4state = 0;

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
    stateOfLight[flickerOrder[i]] = 's';
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
    

    // check the switches:
    
    if (flickerState == LOW)   {
      //flickerDance();
      //flickerDance2();
      /* //need this for flickerDance3
        if ((flickerCount > numLights) || flickerCount == 0) {
        //Serial.println("flickerCount at 0, reset");
        flickerCount = 0;
        }
      */
      //flickerCount = flickerDance3(flickerCount);
      flickerCount = flickerDance4();

    }



    if ( flicker4State == LOW) {
      flicker4(0);

    }

    if (flicker4OffState == LOW) {
      flicker4Off();
      previousFlicker4state = 1 ; // invert the state
    }

    if ((flicker4OffStateBack == LOW) && (previousFlicker4state == 1) ){
      previousFlicker4state = 0 ; // invert the state
      flicker4OffBack();
    }

   



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

} //----------- of main loop !!!!!!!!!!!!!!!!!!!!!!!!!!!!!



int flickerDance4() {  //BUGGGGYYYYYYYY
  // flick the light for a bit before moving to next one
  // keeps rotating between 4
  int l_ = 0;
  // pick interval to be quiet
  int rando = random(0, 10);
  if (rando > 6) {
    //****** CHANGE QUIET LENGTHS HERE:
    flickerInterval_quiet = int(random(2000, 5000));  // pick a delay interval before nxt light
    Serial.println("shorter quiet period");
  }
  else {
    //****** CHANGE QUIET LENGTHS HERE:
    flickerInterval_quiet = int(random(10000, 15000));
    Serial.println("longer quiet period");
  }

  flickerInterval_big = int(random(5000, 10000)); // flicker duration

  startFlickerMillis = millis(); // initial time stamp

  // while we are less than both the pause and the flicker interval:

  while (millis() - startFlickerMillis < (flickerInterval_quiet + flickerInterval_big ) ) {
    // keep flickering our light

    while (millis() - startFlickerMillis < flickerInterval_big) {
      flicker(flickerOrder[l_]);
    } //while
    //Serial.println("done flickering");
    //Serial.println("still waiting for quiet to be done");

  }// while

  Serial.print("number: " ); Serial.println(l_);
  Serial.print("Light: " ); Serial.println(flickerOrder[l_]);

  if (l_ > 4) {
    l_ = 0;
  }
  else {
    l_++;
  }

  return l_;

}

int flickerDance3(int l_) {
  // flick the light for a bit before moving to next one
  // keeps rotating

  // pick interval to be quiet
  int rando = random(0, 10);
  if (rando > 6) {
    //****** CHANGE QUIET LENGTHS HERE:
    flickerInterval_quiet = int(random(2000, 5000));  // pick a delay interval before nxt light
    Serial.println("shorter quiet period");
  }
  else {
    //****** CHANGE QUIET LENGTHS HERE:
    flickerInterval_quiet = int(random(12000, 20000));
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
      tlc.write();
    }

    delay(flickerDelay1[i]);
    //for (int j = numL; j < (numL + 4); j++) {
    for (int j = 0; j <  4; j++) {
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


  else {
    // allLevel(torch);

  }

}


