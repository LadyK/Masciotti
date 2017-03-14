#include "Adafruit_TLC59711.h"
#include <SPI.h>

// How many boards do you have chained?
#define NUM_TLC59711 1

#define POT A0

#define data   11
#define clock  13

Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, clock, data);
//Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711);


/*
#define SPARKLE 2
#define UP 7
#define HEART 4
#define PWM 3
*/

#define demoLED 9
uint32_t maxx = 65535;

uint32_t miniMaxx = 32700;
uint32_t startLevel = 0;
//re-set this with new plug in
int numLights = 9;  //Number of lights we are controlling
uint32_t level_increment = maxx / numLights;  // divide the num of lights by the max, to evenly distribute
uint32_t currentMax = level_increment + startLevel;

//int randLights[] = {3, 6, 2, 4, 1, 5, 0};
int randLights[] = {3, 6, 2, 4, 1, 7, 0, 5, 8};

int flickerPts[5];
//int delayPts[5];
int delayPts[] = {50, 100, 15, 200, 50};
int delayPts2[] = {10, 50, 8, 100, 25};
long randNumber;

// re-set this with alt plug in

//int lightSpiral[] = {
//  3, 0, 1, 4, 7, 6
//};


int lightSpiral[] = {
  4, 3, 5, 6, 2, 1, 7, 8, 0
};

// referencing Paul Badger's Heartbeat sketch ex @ http://playground.arduino.cc/Main/HeartbeatSketch

long heartBeatArray[] = {
  50, 100, 15, 1200
};

int hbeatIndex = 1;   // this initialization is important or it starts on the "wrong foot"
long prevMillis;


byte flickerDelay[] = {100, 2000, 123, 223, 825, 400, 80};
byte flickerDelay1[40];
byte flickerDelay2[40];
byte flickerDelay3[40];
byte flickerDelay4[40];

byte flickerDelayA[40];
byte flickerDelayB[40];
byte flickerDelayC[40];
byte flickerDelayD[40];

byte flickerOrder[] = {0, 5, 7, 9, 2, 4, 3, 1, 8, 6};

// pwm pace is largely set here
long range[] = { 255, 257, 771, 1285, 3855, 4369}; //1, 3, 5, 15, 17, 51, 85,  ..13107, 21845, 65535  // take out 255??
int rangeLength = 6;

char record[9];
bool leave = 0;
int changer = numLights - 1;

unsigned long crazyNum = 4024967295;
char limit;

unsigned long timeLimit = 5000;
unsigned long lastStamp;
unsigned long lastTime = 0;

void setup() {
  Serial.begin(9600);
  //pinMode(demoLED, OUTPUT);
  tlc.begin();
  tlc.write();

  // seed the array with intial values
  for (int i = 0; i < 5; i++) {
    //flickerPts[i] = random(0, 150);
    //delayPts[i] = random(50, 1000);
  }


  // create an array which stores the state of the light; which behavior it's assigned

  // set all the records to flickering
  for (int i = 0; i < numLights; i++) {
    record[i] = 'f';
  }
  //Serial.println("all lights will be flickering");
  limit = 0;
  lastStamp = millis();

  // start off
  for (int i = 0; i < numLights; i ++) {
    tlc.setPWM(i, 0);
  }
  tlc.write();
  delay(2000);
}

void loop() {
  //allFastBrightTwitchvoid(1.0);
  //sparkle_(100, maxx);
  //slowRise_all();
  //two_glitchFlicker();
  //heartBeat(1.5);
  //all_glitch();
  //frustration();
  //flickerBunchA(1.0, 5, maxx);
  //delay(2000);


  /* pulse not working
  // when powered via usb, all lights light
  // when through a wall wort light 1 and 7 don't light
  int pot_r = analogRead(POT);
  Serial.print("pot is: "); Serial.println(pot_r);
  uint32_t pace = map(pot_r, 0, 1023, 0, rangeLength);
  Serial.println(pace);
  pwm_all(range[pace]);
   */
  //************************************

  // 1. All lights on medium level
  for (int i = 0; i < numLights; i ++) {
    tlc.setPWM(i, miniMaxx);
  }
  tlc.write();
  //delay(5000);

  /* the lights struggle to come on. I see this as flickering, warbling -
   * you can see the struggle and they may seem like they're on and OK for a second or
   * two and then burn out. And it starts again.
   *
   * This could happen one by one - each bulb on a different schedule - out of synch with
   * the others - we see the struggle, struggle, struggle, coming on flickering, and burning
   * out. It's an endless cycle of that.
   *
   */

  // 2. one starts to flicker:

  // after some time:
  if (millis() - lastStamp > 10000 && (lastTime == 0)) {
    // flicker 1st one
    Serial.print("Limit is: ");
    Serial.println(limit);
    flickerGroup(limit);
    lastTime = 1;  // let's not come here again
    lastStamp = millis(); //new time stamp
  }

  // every so often
  // 2.c make a new light be a flickerer
  // regardless:
  else if ( millis() - lastStamp > timeLimit && (lastTime == 1)) {
    Serial.println(limit);
    if (limit < 4) {
      limit++;
    }
    else if (limit >= 4) {
      limit = 0;
    }
    Serial.print("Limit is: ");
    Serial.println(limit);
    flickerGroup(limit);
    lastStamp = millis();
  }


  /*
    // do this for a certain amount of times:
    for (int j = 0; j < 1; j++) {

      //go through all the lights and throw in a few random ones along the way
      for (int i = 0; i < numLights; i++) {
        //flickerArchMany(i, 1.0, 4, 5000, maxx); // turns on this particular light, as well as a random one
        flickerArch(i, 1.0, 4, 5000, maxx);
        delay(200);
      }
      Serial.println(j);
    }

  //*/

  /*
   *  flickering and blitzing out would start to get in unison and assume the heartbeat throb
   *  - the calming setting - a few would get in synch - then a few others - until all of them
   *  are in synch - they stay in synch for a while
   *
   *
   *
   */



  /*

    int cntdwn = 20000;
    long switchTime = millis();

    int laps = 0;
    long startTime = millis();
    // how many times are we going to go? must be longer than the switchTime
    while ( millis() - startTime < cntdwn) {
      Serial.println("top of while loop");

      /*
          //loop through each light's record and go accordingly
          for (int i = 0; i < numLights; i++) {
            Serial.println("inside for-loop");
            if (record[i] == 'f') {
              flickerArch_1(randLights[i], 1.0, 4, 5000, maxx);
            }
            else if (record[i] == 'p') {
              Serial.println("we are pwm'ing a light");
              int pot_r = analogRead(POT);
              //Serial.print("pot is: "); Serial.println(pot_r);
              uint32_t  pace = map(pot_r, 0, 1023, 0, rangeLength - 1); // pot max is 872
              pwm_some(range[pace], randLights[i]);
            }
          } // all the lights to action
      */


  /*
      int pot_r = analogRead(POT);
      Serial.print("pot is: "); Serial.println(pot_r);
      uint32_t  pace = map(pot_r, 0, 1023, 0, rangeLength - 1); // pot max is 872
      trans(pace, changer);

      // every so often, say 4 seconds, change the record on a light

      Serial.println("has enough time passed?");
      if (millis() - switchTime > 4000) {
        Serial.println("YES....changing a record");
        // change the record on one light
        record[changer] = 'p';
        changer--;
        Serial.println(changer);
        // re-set the clock
        switchTime = millis();
        if (changer <= 0) {
          changer = numLights - 1;
          leave = 1;
          break;
        }

        Serial.println("leaving record change");
      } // end of flipping record

      //laps++;
      Serial.println("oneLap, bottom of While loop");
    } // end of lap while

  */

  /*
  Serial.println("done");
  delay(5000);
  //* /
  // /*
  Serial.println("transitioning toward heartbeat");
  long startTime = millis();
  long duration = 10000;
  while (millis() - startTime < duration) {
    /*
    int pot_r = analogRead(POT);
    Serial.print("pot is: "); Serial.println(pot_r);
    uint32_t pace = map(pot_r, 0, 1023, 0, rangeLength - 1); // pot max is 872
    pwm_all(pace);
    */
  //  heartBeat(1.5);
  // }



  //for (int i = 0; i < 8; i++) {
  // tlc.setPWM(i, 0);
  //}
  //tlc.write();
  //delay(3000);
  // */

  //Serial.println("in the heartbeat");
  //for (int i = 0; i < 20; i++) {

  // get a time stamp
  // while (millis() - start time < 10 seconds) {
  // heartBeat(1.0);

  //}




  //be off
  /*

  //tlc.setPWM(4, 0);
  //tlc.write();
  delay(5000);

  // try to turn on:
  // flicker(4, 1.0, 4, 5000);
  //flickerArch(1, 1.0, 4, 5000, maxx);


  /*
  // turn all the lights off
  delay(2000);
  for(int i = 0; i < numLights; i ++){
    tlc.setPWM(i, 0);
  }
  tlc.write();
  */

  //Serial.println("end of loop");


}  // end of loop

void  flickerGroup(char limit_) { // flicker the lights
  // 2. one starts to flicker:
  // 2.a - Seed random lengths to be on and off
  for (int i = 0; i < 40; i++) {
    flickerDelay1[i] = int(random(0, 80));  // how long to be on
    flickerDelay2[i] = int(random(0, 80));
    flickerDelay3[i] = int(random(0, 80));
    flickerDelay4[i] = int(random(0, 80));
  }
  for (int i = 0; i < 40; i++) {
    flickerDelayA[i] = int(random(0, 150));  // how long to be off for some of them
    flickerDelayB[i] = int(random(0, 150));
    flickerDelayC[i] = int(random(0, 150));
    flickerDelayD[i] = int(random(0, 150));
  }

  char j = 0;
  //Serial.println("starting loop");
  //for ( int i = 0; i < 312500000; i= i * 50.3) {  //nice flicker with this
  for (char i = 0; i < sizeof(flickerDelay2) / sizeof(char); i++) {
    // another for loop here to go through all of the flicker lights
    //for( j = limit; j <sizeof(flickerOrder)/sizeof(char); j++){
    while ( j <= limit_) {
      tlc.setPWM(flickerOrder[j], miniMaxx);  //level
      j++;
      Serial.println(j);
    }// end for loop
    j = 0;
    tlc.write();
    delay(flickerDelay1[i]);  //delay  on

    while ( j <= limit_) {
      //  loop again for all the flicker lights to go off
      tlc.setPWM(flickerOrder[j], 0);
      j++; // light the rest of the ones we are supposed to be lighting
      Serial.println(j);
    }// end for loop
    j = 0;
    tlc.write();
    // delay off:
    if (i % 3 == 0) {  // every so often, have a larger delay
      delay(flickerDelayA[i]);
    } else {
      delay(5);
    }
  }
  //Serial.println("ending");
}

void flickerBunchA(float tempo, int t, uint32_t level) {
  for (int j = 0; j < t; j++) {
    for (int i = 0; i < sizeof(lightSpiral) / sizeof(int); i++) {
      tlc.setPWM(i, level);
    }
    tlc.write();
    delay(delayPts2[j]);
    for (int i = 0; i < sizeof(lightSpiral) / sizeof(int); i++) {
      tlc.setPWM(i, 0);
    }
    tlc.write();
    delay(delayPts2[j]);
  }
  delay(2000);
}

void trans(uint32_t pace_, int c) {
  //int pwm_limit = numLights - c; // c will be how many lights have been changed to pwm
  uint32_t last;
  //uint32_t maxx = 65535;
  //up:
  //uint32_t level_increment;
  // put a pot up to pick one of these values from an array mapped to it's dial
  // 1, 3, 5, 15, 17, 51, 85, 255, 257, 771, 1285, 3855, 4369, 13107, 21845, 65535


  for (uint32_t level_increment = 0; level_increment <= 65535; level_increment += pace_) {  // for (uint32_t level_increment = 0; level_increment <= 65535; level_increment += 255) { works
    Serial.println("inside increase");

    // go through the array backwards:
    for (int i = (sizeof(randLights) / sizeof(int)); i > c; i--) { // only pulse the ones that are changed
      tlc.setPWM(lightSpiral[i], level_increment);
    }
    //tlc.write();
    // flicker others
    int beatIndex = 1;   // this initialization is important or it starts on the "wrong foot"
    if ((millis() - prevMillis) > (long)(flickerPts[beatIndex] * 1.0)) {
      beatIndex++;
      if (beatIndex > 5) beatIndex = 0;

      if ((beatIndex % 2) == 0) {
        // set all the others to flicker
        for (int i = 0; i < c; i++) { // how many lights we are going to do
          tlc.setPWM(lightSpiral[i], 5000);
        }
        tlc.write();
      }
      beatIndex++;
    }
    //tlc.write(); // time here and all at once w/o delay below;  w/delay below time btwn ea step
    //delay(10); // time btwn each stage
    //Serial.println(level_increment);
    last = level_increment;
  }
  delay(500); // ??
  Serial.println("all off now");
  for (int i = 0; i < numLights; i++) {
    tlc.setPWM(lightSpiral[i], 0);
  }
  tlc.write();
  delay(2000);
  Serial.println("now taking them down");
  //down
  //uint32_t level_incrementb;
  for (uint32_t level_incrementb = last; level_incrementb > 0; level_incrementb -= pace_) {
    //Serial.println("inside decrease");
    // go through the array backwards:
    for (int i = sizeof(randLights) / sizeof(int); i > c; i--) { // only pulse the ones that are changed
      tlc.setPWM(lightSpiral[i], level_incrementb);
    }
    for (int i = 0; i < c; i++) { // how many lights we are going to do
      tlc.setPWM(lightSpiral[i], 0);
    }
    tlc.write(); // time here and all at once w/o delay below; w/delay below time btwn ea step
    //delay(10); // time btwn each stage
    //Serial.println(level_incrementb);
  }
  hbeatIndex++;
}


void flickerArch_1(int light, float temp, int t, uint32_t level1, uint32_t level2) {

  // try to turn on:
  flicker(lightSpiral[light], temp, t, level1);
  // turn on
  tlc.setPWM(lightSpiral[light], level2);

  tlc.write();
  delay(2000);
  // briefly turn off
  tlc.setPWM(lightSpiral[light], 0);
  tlc.write();
  delay(100);

}

// flickers on and off, then flickers off and on....
void flickerArch(int light, float temp, int t, uint32_t level1, uint32_t level2) {
  //tlc.setPWM(lightSpiral[light], 0);
  int randie = int(random(sizeof(lightSpiral) / sizeof(int))); //
  // tlc.write();
  //delay(5000);
  // try to turn on a little dim:
  flicker(lightSpiral[light], temp, t, level1);
  // turn on brighter
  tlc.setPWM(lightSpiral[light], level2);

  // turn another random one on
  flicker(lightSpiral[randie], temp, t, level1);
  tlc.setPWM(lightSpiral[randie], level2);

  tlc.write();
  delay(2000);
  // briefly turn off
  tlc.setPWM(lightSpiral[light], 0);
  tlc.write();
  delay(100);
  //flicker briefly brighter
  flicker(lightSpiral[light], temp, t + 1, level2);

  tlc.setPWM(lightSpiral[randie], 0);
  tlc.write();
  delay(100);
  flicker(lightSpiral[randie], temp, t + 1, level2);



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


// all on and tight fast flicker
void all_glitch() {
  //faster
  //uint32_t maxx = 65535;
  uint32_t level_increment = 0;
  if (level_increment == 0 && level_increment <= 65535) {
    //for (level_increment = 0; level_increment <= 65535; level_increment += 255) {
    for (int i = 0; i < 9; i++) {
      tlc.setPWM(i, level_increment);

      //delay(100);
    }
    tlc.write(); // time here and all at once w/o delay below;  w/delay below time btwn ea step
    //delay(10); // time btwn each stage
    //Serial.println(level_increment);
    level_increment += 255;
  }

  // tlc.write();
  //delay(100); // at peak to hold
  uint32_t level_incrementb = 65535;
  if (level_incrementb == 65535 && level_incrementb >= 0) {
    //for (level_incrementb = 65535; level_incrementb >= 0; level_incrementb -= 255) {
    Serial.println("inside decrease");
    for (int i = 0; i < 9; i++) {
      tlc.setPWM(i, level_incrementb);

      //delay(100);
    }
    tlc.write(); // time here and all at once w/o delay below; w/delay below time btwn ea step
    //delay(10); // time btwn each stage
    level_incrementb -= 255;
  }
  //tlc.write();

  delay(100);  // at peak to hold

}


// irregular blink pattern btwn 4; turn down the pot and knightrider across space, one at a time
void inputBlinkRyder() {

  // uint32_t var;
  int pot_r = analogRead(POT);
  Serial.println(pot_r);
  int d = map(pot_r, 0, 1023, 0, 200);  // pot max is 1023
  ///*
  int channela = ceil(random(0, numLights - 1));
  int channelb = ceil(random(0, numLights - 1));
  tlc.setPWM(channela, maxx);
  tlc.setPWM(channelb, maxx);
  tlc.write();
  delay(d);
  tlc.setPWM(channela, 0);
  tlc.setPWM(channelb, 0);
  delay(d);
  //*/
  if (d < 50) {
    knightRider();
  }

}

void knightRider() {
  int timer = 30;
  for (int count = 0; count < 9; count++) {
    tlc.setPWM(count, maxx);
    delay(timer);
    tlc.write();
    tlc.setPWM(count + 1, maxx);
    tlc.write();
    delay(timer);
    tlc.setPWM(count, 0);
    tlc.write();
    delay(timer * 2);
  }
  for (int count = 9; count > 0; count--) {
    tlc.setPWM(count, maxx);
    tlc.write();
    delay(timer);
    tlc.setPWM(count - 1, maxx);
    tlc.write();
    delay(timer);
    tlc.setPWM(count, 0);
    tlc.write();
    delay(timer * 2);
  }
}
void two_glitchFlicker() {

  for (int i = 0; i < 7; i++) { // maybe a longer range than 7
    //uint32_t range = map(flickerDelay[i], 0, 255, 0, maxx);
    tlc.setPWM(0, maxx);  // randomly pick 2 lights for each call
    tlc.setPWM(7, maxx);
    tlc.write();
    delay(flickerDelay[i]);  // want this delay to be sporadically longer
    tlc.setPWM(0, 0); // off
    tlc.setPWM(7, 0); // off
    tlc.write();
    delay(flickerDelay[random(sizeof(flicker - 1))]);
  }



}

void slowRise_all() {
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
  //startLevel = 0;

  /*
    delay(1000);
     while (c < numLights) { // for every light
      if (startLevel >= currentMax * 0.5) {
        tlc.setPWM(c, startLevel);
        tlc.write();
      }
      if (startLevel < currentMax * 0.5) {
        tlc.setPWM(c + 1, startLevel);
        tlc.write();
      }
      startLevel = startLevel - 100;


      c++; // move to the next light?
    }// while
    */

}

// across all bulbs. strong, rather than flicker
void heartBeat(float tempo) {
  //a bit delayed individually across all the lights. cascade

  if ((millis() - prevMillis) > (long)(heartBeatArray[hbeatIndex] * tempo)) {
    hbeatIndex++;
    if (hbeatIndex > 3) hbeatIndex = 0;
    if ((hbeatIndex % 2) == 0) {

      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {
        //digitalWrite(demoLED, HIGH);
        tlc.setPWM(lightSpiral[LEDpin], maxx * .5);
        // tlc.write();
        //delay((int)heartBeatArray[hbeatIndex]); // D
      }
      tlc.write();
      delay((int)heartBeatArray[hbeatIndex]);  // D
      // }  // toggle this for all on off but a bit out of synch  // <-- commented this
      //delay((int)heartBeatArray[hbeatIndex]); // <-- commented this
      // digitalWrite(12, LOW);
      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {
        //digitalWrite(demoLED, LOW);
        tlc.setPWM(lightSpiral[LEDpin], 0);
        //tlc.write();
        // delay((int)heartBeatArray[hbeatIndex]); // C D   <-- commented this
      }
      tlc.write();
      // hbeatIndex++;  // C D  <-- commented this
      // prevMillis = millis(); //C D  <-- commented this
      // delay((int)heartBeatArray[hbeatIndex]);
    }
    //}
    hbeatIndex++; // <-- uncomment
    prevMillis = millis();  //<-- uncomment
  } //<-- uncomment
}

// one beat across all bulbs; cascade
void heartBeat_other(float tempo) {
  //a bit delayed individually across all the lights. cascade

  if ((millis() - prevMillis) > (long)(heartBeatArray[hbeatIndex] * tempo)) {
    hbeatIndex++;
    if (hbeatIndex > 3) hbeatIndex = 0;
    if ((hbeatIndex % 2) == 0) {

      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {
        digitalWrite(demoLED, HIGH);
        tlc.setPWM(lightSpiral[LEDpin], maxx * .5);
        tlc.write();
        //delay((int)heartBeatArray[hbeatIndex]); // D
        //   }
        //tlc.write();
        //delay((int)heartBeatArray[hbeatIndex]);  // D
        //  }  // toggle this for all on off but a bit out of synch
        //delay((int)heartBeatArray[hbeatIndex]);
        // digitalWrite(12, LOW);
        // for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {
        digitalWrite(demoLED, LOW);
        tlc.setPWM(lightSpiral[LEDpin], 0);
        tlc.write();
        //delay((int)heartBeatArray[hbeatIndex]); // C D
        // }
        // tlc.write();
        // hbeatIndex++;  // C D
        // prevMillis = millis(); //C D
        // delay((int)heartBeatArray[hbeatIndex]);
      }
    }
    hbeatIndex++;
    prevMillis = millis();
  }
}

// all of them going, slightly off. almost unison, but a bit off
void heartBeat_allButALittleOff(float tempo) {
  //a bit delayed individually across all the lights. cascade

  if ((millis() - prevMillis) > (long)(heartBeatArray[hbeatIndex] * tempo)) {
    hbeatIndex++;
    if (hbeatIndex > 3) hbeatIndex = 0;
    if ((hbeatIndex % 2) == 0) {

      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {
        digitalWrite(demoLED, HIGH);
        tlc.setPWM(lightSpiral[LEDpin], maxx * .5);
        tlc.write();
        delay((int)heartBeatArray[hbeatIndex]);
      }

      //delay((int)heartBeatArray[hbeatIndex]);
      // digitalWrite(12, LOW);
      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {
        digitalWrite(demoLED, LOW);
        tlc.setPWM(lightSpiral[LEDpin], 0);
        tlc.write();
        delay((int)heartBeatArray[hbeatIndex]);
      }

    }
    hbeatIndex++;
    prevMillis = millis();
  }

}


void heartBeat_weird(float tempo) {
  //a bit delayed individually across all the lights. cascade

  if ((millis() - prevMillis) > (long)(heartBeatArray[hbeatIndex] * tempo)) {
    hbeatIndex++;
    if (hbeatIndex > 3) hbeatIndex = 0;

    if ((hbeatIndex % 2) == 0) {

      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {

        //digitalWrite(12, HIGH);
        tlc.setPWM(lightSpiral[LEDpin], maxx);
        tlc.write();
        delay((int)heartBeatArray[hbeatIndex]);
        // digitalWrite(12, LOW);
        tlc.setPWM(lightSpiral[LEDpin], 0);
        tlc.write();


      }
    }
    hbeatIndex++;
    // Serial.println(hbeatIndex);
    prevMillis = millis();

  }
}

// flickers on and off, then flickers off and on....
void flickerArchMany(int light, float temp, int t, uint32_t level1, uint32_t level2) {
  //tlc.setPWM(lightSpiral[light], 0);
  int randie1 = int(random(sizeof(lightSpiral) / sizeof(int))); //
  // tlc.write();
  //delay(5000);
  // try to turn on a little dim:
  flicker(lightSpiral[light], temp, t, level1);
  // turn on brighter
  tlc.setPWM(lightSpiral[light], level2);
  tlc.write();

  // turn another random one on
  flicker(lightSpiral[randie1], temp, t, level1);
  tlc.setPWM(lightSpiral[randie1], level2);
  tlc.write();

  //from here
  int randie2 = int(random(sizeof(lightSpiral) / sizeof(int)));
  flicker(lightSpiral[randie2], temp, t, level1);
  tlc.setPWM(lightSpiral[randie2], level2);
  tlc.write();

  int randie3 = int(random(sizeof(lightSpiral) / sizeof(int)));
  flicker(lightSpiral[randie3], temp, t, level1);
  tlc.setPWM(lightSpiral[randie3], level2);
  // to here

  tlc.write();
  delay(2000);
  // briefly turn off
  for (int i = 0; i < numLights; i++) {
    tlc.setPWM(lightSpiral[i], 0);
  }
  tlc.write();
  delay(100);
  //flicker briefly brighter
  flicker(lightSpiral[light], temp, t + 1, level2);

  tlc.setPWM(lightSpiral[randie1], 0);
  tlc.write();
  delay(100);
  flicker(lightSpiral[randie1], temp, t + 1, level2);

  tlc.setPWM(lightSpiral[randie2], 0);
  tlc.write();
  delay(100);
  flicker(lightSpiral[randie2], temp, t + 1, level2);

  tlc.setPWM(lightSpiral[randie3], 0);
  tlc.write();
  delay(100);
  flicker(lightSpiral[randie3], temp, t + 1, level2);



}

// flicker on and off for t number of times. Ends off
void flicker(int light, float tempo, int t, uint32_t level) {
  //Serial.println("Flicker");

  //delay(2000);

  int beatIndex = 1;   // this initialization is important or it starts on the "wrong foot"
  if ((millis() - prevMillis) > (long)(flickerPts[beatIndex] * tempo)) {
    beatIndex++;
    if (beatIndex > 5) beatIndex = 0;

    if ((beatIndex % 2) == 0) {


      for (int i = 0; i < t; i++) {   // flicker for "t" times
        //  for (int j = 0; j <= 10; j++) {
        //analogWrite(demoLED, 255);
        tlc.setPWM(light, level);
        tlc.write();  //MUST BE CALLED TO DISPLAY CHANGES
        // }
        //analogWrite(demoLED, flickerPts[i]);
        //flickerPts[i] = random(10, 100);
        // for (int j = 0; j <= 10; j++) {
        delay(delayPts2[i]);
        // }
        //delayPts[i] = random(50, 500);
        // for (int j = 0; j <= 10; j++) {
        // analogWrite(demoLED, 0);
        tlc.setPWM(light, 0);
        tlc.write();  //MUST BE CALLED TO DISPLAY CHANGES
        // }
        // for (int j = 0; j <= 10; j++) {
        delay(delayPts2[i]);
        // }
      }
    }
    beatIndex++;
    // Serial.println(hbeatIndex);
    prevMillis = millis();
  }
  //Serial.println("Back to normal");
}


void sparkle_(int d, uint32_t level) {
  for (int i = 0; i <= 12; i++) {
    if (i % 2 == 0) {
      tlc.setPWM(i, level);
    }
    else {
      tlc.setPWM(i, 0);
    }
    tlc.write();  //MUST BE CALLED TO DISPLAY CHANGES
  }
  delay(d);
  for (int i = 0; i <= 12; i++) {
    if (i % 2 == 0) {
      tlc.setPWM(i, 0);
    }
    else {
      tlc.setPWM(i, level);
    }
    tlc.write();  //MUST BE CALLED TO DISPLAY CHANGES
  }
  delay(d);

}

void allFastBrightTwitchvoid(float tempo) {
  //a bit delayed individually across all the lights. cascade

  if ((millis() - prevMillis) > (long)(heartBeatArray[hbeatIndex] * tempo)) {
    hbeatIndex++;
    if (hbeatIndex > 3) hbeatIndex = 0;

    if ((hbeatIndex % 2) == 0) {

      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {
        tlc.setPWM(lightSpiral[5], maxx); // ?? not working
        //digitalWrite(12, HIGH);
        tlc.setPWM(lightSpiral[LEDpin], maxx);
        tlc.write();
      }
      delay((int)heartBeatArray[hbeatIndex]);
      // digitalWrite(12, LOW);
      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {

        tlc.setPWM(lightSpiral[LEDpin], 0);
        tlc.write();
      }

    }
  }
  hbeatIndex++;
  // Serial.println(hbeatIndex);
  prevMillis = millis();


}

// old code: can't remember what this is for;
// something building in sensation, moving through states
//2 lights. one rises in value and repeats
void frustration() {
  uint32_t level_1, level_2, level_3, level_2max, level_3max;
  bool flag1 = 0;
  bool flag2 = 0;
  bool flag3 = 0;
  for (int c = 0; c < 3; c++) {

    if (flag1 == 0 && flag2 == 0) {
      //light 1st led:
      tlc.setPWM(c, startLevel);
      tlc.write();
      startLevel = startLevel + 100; // increase
    }
    //if it's beyond a level:
    if (startLevel > maxx * 0.2) {
      flag1 = 1;  // flip the flag
      level_1 = startLevel; // roll over the value
    }
    startLevel = startLevel + 100;
  }// for loop
  Serial.print("flag1 is: "); Serial.println(flag1);
} // frustration

