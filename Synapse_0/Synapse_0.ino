// to flip between states: pin, varables for states + debounce

#include "Adafruit_TLC59711.h"
#include <SPI.h>

// How many boards do you have chained?
#define NUM_TLC59711 1

#define data   11
#define clock  13

Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711, clock, data);
//Adafruit_TLC59711 tlc = Adafruit_TLC59711(NUM_TLC59711);

#define SPARKLE 2
#define UP 7   // check <--- this is currently fast pwm all
#define HEART 4  // check and currently fucked glitchy all
#define MISF 3  // knightryder misfire   
#define OFF 8
#define POT A0
#define GLITCH 6

int thrown = 0;
bool s;
bool u;
bool h;
bool misf;
bool g;
bool s_state = 0;
bool u_state = 0;
bool h_state = 0;
bool misf_state = 0;
bool pwm_state = 0;
bool g_state = 0;
bool off_ = 0;
bool xx = 0; // this is what helps us break out of loops
/*
long heartBeatArray[] = {
  50, 100, 15, 1200
};
*/

int rangeLength = 6;

// pwm pace is largely set here
long range[] = { 255, 257, 771, 1285, 3855, 4369}; //1, 3, 5, 15, 17, 51, 85,  ..13107, 21845, 65535  // take out 255??
/*long heartBeatArray[] = {
  25, 50, 8, 600
};
*/
long heartBeatArray[] = {
  50, 100, 16, 1200
};

byte flicker[] = {100, 2000, 123, 223, 825, 400, 80};

//int lightOrder[] = 

int hbeatIndex = 1;   // this initialization is important or it starts on the "wrong foot"

long prevMillis;
byte leds = 0;

int numLights = 12;  //Number of lights we are controlling

/*
int SER_Pin = 8;   //pin 14 on the 75HC595  (Serial/Data Pin) Input for the next pin that gets shifted in.
int RCLK_Pin = 5;  //pin 12 on the 75HC595 (Register Clock/Latch Pin) Needs to be pulled high to set the
// output to the new shift register values,
//This must be pulled high directly after SRCLK has gone LOW again.
int SRCLK_Pin = 6; //pin 11 on the 75HC595  (Serial Clock) When this pin is pulled high, it will shift the register.

int outputEnablePin = 10;  // pin 13 on 74HC595

//How many of the shift registers - change this
#define number_of_74hc595s 1

//do not touch
#define numOfRegisterPins number_of_74hc595s * 8
*/
//boolean registers[numOfRegisterPins];

uint32_t maxx = 65500 / 2; //65535;
uint32_t startLevel = 0;

void setup() {
  /*
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  */
  // pinMode(12, OUTPUT);
  Serial.begin(9600);
  pinMode(SPARKLE, INPUT_PULLUP);
  digitalWrite(SPARKLE, HIGH);
  pinMode(UP, INPUT_PULLUP); // pwm
  digitalWrite(UP, HIGH);
  pinMode(HEART, INPUT_PULLUP);
  digitalWrite(HEART, HIGH);
  pinMode(MISF, INPUT_PULLUP);
  digitalWrite(MISF, HIGH);
  pinMode(OFF, INPUT_PULLUP);
  digitalWrite(OFF, HIGH);
  pinMode(GLITCH, INPUT_PULLUP);
  digitalWrite(GLITCH, HIGH);

  //pinMode(outputEnablePin, OUTPUT);
  //analogWrite(outputEnablePin, 0);  // set this to ground for digital output

  //reset all register pins
  // clearRegisters();
  // writeRegisters();
  //Serial.println(numOfRegisterPins);
  //pinMode(10, OUTPUT);
  tlc.begin();
  tlc.write();
  //digitalRead(SPARKLE) = s_state;
  //s_state = digitalRead(SPARKLE);
  Serial.print("sparkle at startup is: "); Serial.println(s_state);
}


void loop() {
  //Serial.print("Blue is: ");
  //Serial.println(digitalRead(UP));
  //knightRider();
  /*
  colorWipe(65535, 0, 0, 100); // "Red" (depending on your LED wiring)
  delay(200);
  colorWipe(0, 65535, 0, 100); // "Green" (depending on your LED wiring)
  delay(200);
  colorWipe(0, 0, 65535, 100); // "Blue" (depending on your LED wiring)
  delay(200);
  */
  //rainbowCycle(5);
  //pwm_all();
  //sparkle_(100, maxx);
  //heartBeat(1.0);
  /*
  Serial.print("s_state is: ");
  Serial.println(s_state);
  delay(1000);
  //s_state = digitalRead(SPARKLE);
  Serial.print("s_state is: ");
  Serial.println(s_state);
  Serial.println("here");
  Serial.print("pin is: "); Serial.println(digitalRead(SPARKLE));
  */

  if (digitalRead(SPARKLE) == LOW) {
    s_state = 1;
    Serial.println("sparkle");
  }
  else if (digitalRead(HEART) == LOW) {
    h_state = 1;
  }
  else if (digitalRead(UP) == LOW) {
    u_state = 1;
  }

  else if (digitalRead(MISF) == LOW) {
    misf_state = 1;
  }
  else if (digitalRead(GLITCH) == LOW) {
    g_state = 1;
  }

  else if (digitalRead(OFF) == LOW) {  // off button
    off_ = 1;

  }



  while (s_state == 1 && xx == 0) {  //                                      sparkle
    Serial.println("inside while loop");
    sparkle_(50, maxx);
    //sparkle_(100, maxx / 2); //maxx = 65535
    u = digitalRead(UP);
    h = digitalRead(HEART);
    misf = digitalRead(MISF);
    g = digitalRead(GLITCH);
    off_ = digitalRead(OFF);

    //Serial.print("XX is: "); Serial.println(xx);
    //Serial.print("U is: "); Serial.println(u);
    //Serial.print("H is: "); Serial.println(h);
    if (u == LOW || h == LOW || misf == LOW || g == LOW || off_ == LOW) {  // if any of the other buttons are pressed, break
      //clearRegisters();
      //writeRegisters();
      for (int i = 0; i < numLights; i++) {
        tlc.setPWM(i, 0);
      }
      tlc.write();
      xx = 1; // so we can exit loop
      s_state = 0;  //so we can get back in, if need be


    }
  }

  xx = 0; // flip back

  while (u_state == 1 && xx == 0) {                                       //up PWM ALL Ascention
    //ascend(100);
    // take reading from pot
    int pot_r = analogRead(POT);
    Serial.print("pot is: "); Serial.println(pot_r);
    uint32_t  pace = map(pot_r, 0, 872, 0, rangeLength - 1); // pot max is 872
    Serial.print("pot is at this array spot: ");
    Serial.println(pace);
    Serial.println(range[pace]);
    // map that to array
    // pass indice to function
    pwm_all(range[pace]);
    s = digitalRead(SPARKLE);
    h = digitalRead(HEART);
    //misf = digitalRead(MISF);
    g = digitalRead(GLITCH);
    off_ = digitalRead(OFF);

    Serial.println("PWM");
    if (u == LOW || h == LOW || misf == LOW || g == LOW || off_ == LOW){  // if any of the other buttons are pressed, break
      xx = 1; // so we can exit loop
      u_state = 0;  //so we can get back in, if need be
    }
  }

  xx = 0;



  while (h_state == 1 && xx == 0) {                             //heartbeat
    float pot_r = analogRead(POT);
    //constrain(pot_r, 256.0, 872.0);
    //float temp = map(pot_r, 256.0, 872.0, 0.0, 300.0);
    Serial.print("pot is: "); Serial.println(pot_r);
    float  pace = map(pot_r, 0, 872, 0, 300);  // pot max is 872//heartbeat
    //constrain(pot_r, 0.0, 2.0);
    pace = pace * .01;
    Serial.print("pace is: "); Serial.println(pace);
    heartBeat(pace);
    
    s = digitalRead(SPARKLE);
    u = digitalRead(UP);
    misf = digitalRead(MISF);
    g = digitalRead(GLITCH);
    off_ = digitalRead(OFF);

    Serial.println("heartbeat");
    if (u == LOW || h == LOW || misf == LOW || g == LOW || off_ == LOW) {  // if any of the other buttons are pressed, break

      xx = 1; // so we can exit loop
      h_state = 0;  //so we can get back in, if need be
    }
  }

  xx = 0;

  while (misf_state == 1 && xx == 0) {                                        //kightryder
    //frustration();
    knightRider();
    
    s = digitalRead(SPARKLE);
    u = digitalRead(UP);
    h = digitalRead(HEART);
    off_ = digitalRead(OFF);

    Serial.println("knightrider");
    if (u == LOW || h == LOW || misf == LOW || g == LOW || off_ == LOW) {  // if any of the other buttons are pressed, break

      xx = 1; // so we can exit loop
      misf_state = 0;  //so we can get back in, if need be
    }
  }

   while (g_state == 1 && xx == 0) {                                //GLITCH
   // all_glitch();
    //two_glitch();
    two_glitchFlicker();
    s = digitalRead(SPARKLE);
    u = digitalRead(UP);
    h = digitalRead(HEART);
    misf = digitalRead(MISF);
    off_ = digitalRead(OFF);
    
    Serial.println("GLITCHing");
    if (u == LOW || h == LOW || misf == LOW || s == LOW || off_ == LOW) {  // if any of the other buttons are pressed, break
      xx = 1; // so we can exit loop
      g_state = 0;  //so we can get back in, if need be
    }
  }

  xx = 0;

  while (off_ == 1 && xx == 0) {                                                 // all off
    s = digitalRead(SPARKLE);
    u = digitalRead(UP);
    h = digitalRead(HEART);
    misf = digitalRead(MISF);
    g = digitalRead(GLITCH);
    for (int i = 0; i < numLights; i++) {
      tlc.setPWM(i, 0);
    }
    tlc.write();
    Serial.println("OFF");
    if (u == LOW || h == LOW || misf == LOW || g == LOW || off_ == LOW) {  // if any of the other buttons are pressed, break
      xx = 1; // so we can exit loop
      off_ = 0;  //so we can get back in, if need be
    }
  }

  xx = 0;

  delay(250);

} // end loop




/******* Neural Modes ******/
/*
void sparkle(int d) {
  for (int i = 1; i <= 9; i++) {
    if (i % 2 == 0) {
      setRegisterPin(i, HIGH);
    }
    else {
      setRegisterPin(i, LOW);
    }
    writeRegisters();  //MUST BE CALLED TO DISPLAY CHANGES
  }
  delay(d);
  for (int i = 1; i <= 9; i++) {
    if (i % 2 == 0) {
      setRegisterPin(i, LOW);
    }
    else {
      setRegisterPin(i, HIGH);
    }
    writeRegisters();  //MUST BE CALLED TO DISPLAY CHANGES
  }
  delay(d);

}
*/

void two_glitchFlicker() {

  for (int i = 0; i < 7; i++) { // maybe a longer range than 7
    //uint32_t range = map(flicker[i], 0, 255, 0, maxx);
    tlc.setPWM(3, maxx);  // randomly pick 2 lights for each call
    tlc.setPWM(5, maxx);
    tlc.write();
    delay(flicker[i]);  // want this delay to be sporadically longer
    tlc.setPWM(3, 0); // off
    tlc.setPWM(5, 0); // off
    tlc.write();
    delay(flicker[random(sizeof(flicker-1))]);
  }



}


void two_glitch() {
  // uint32_t var;
  int pot_r = analogRead(POT);
  int d = map(pot_r, 0, 872, 0, 200);  // pot max is 872
  int channela = ceil(random(0, numLights - 1));
  int channelb = ceil(random(0, numLights - 1));
  tlc.setPWM(channela, maxx);
  tlc.setPWM(channelb, maxx);
  tlc.write();
  delay(d);
  tlc.setPWM(channela, 0);
  tlc.setPWM(channelb, 0);
  delay(d);

  if (d < 10) {
    knightRider();
  }
}

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



void sparkle_(int d, uint32_t level) {
  Serial.println("We are sparkling");
  for (int i = 0; i <= 12; i++) {
    if (i % 2 == 0) {
      tlc.setPWM(i, maxx);
      // tlc.write();
    }
    else {
      tlc.setPWM(i, 0);
      //tlc.write();
    }
    tlc.write();  //MUST BE CALLED TO DISPLAY CHANGES
  }
  delay(d);
  for (int i = 0; i <= numLights; i++) {
    if (i % 2 == 0) {
      tlc.setPWM(i, 0);
      //tlc.write();
    }
    else {
      tlc.setPWM(i, maxx);
      //tlc.write();
    }
    tlc.write();  //MUST BE CALLED TO DISPLAY CHANGES
  }
  delay(d);

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

void ascend(int d) {
  tlc.setPWM(4, maxx);
  tlc.write();

  delay(100);
  tlc.setPWM(3, maxx);
  tlc.write();
  delay(100);
  tlc.setPWM(5, maxx);
  tlc.write();
  delay(100);
  tlc.setPWM(7, maxx);
  tlc.write();
  delay(100);

  tlc.setPWM(6, maxx);
  tlc.write();
  delay(100);
  tlc.setPWM(4, 0);
  tlc.write();
  delay(100);
  tlc.setPWM(9, maxx);
  tlc.write();
  delay(100);
  tlc.setPWM(3, 0);
  tlc.write();
  delay(100);
  tlc.setPWM(5, 0);
  tlc.write();
  delay(100);
  tlc.setPWM(1, maxx);
  tlc.write();
  delay(100);
  tlc.setPWM(2, maxx);
  tlc.write();
  delay(100);
  tlc.setPWM(8, maxx);
  tlc.write();
  delay(100);
  tlc.setPWM(7, 0);
  tlc.write();
  delay(100);

  tlc.setPWM(6, 0);
  tlc.write();
  delay(100);

  tlc.setPWM(9, 0);
  tlc.write();
  delay(100);

  tlc.setPWM(1, 0);
  tlc.write();
  delay(100);

  tlc.setPWM(2, 0);
  tlc.write();
  delay(100);
  tlc.setPWM(8, 0);
  tlc.write();
  delay(100);

  delay(2000);

}


void heartBeat(float tempo) {
  if ((millis() - prevMillis) > (long)(heartBeatArray[hbeatIndex] * tempo)) {
    hbeatIndex++;
    if (hbeatIndex > 3) hbeatIndex = 0;

    if ((hbeatIndex % 2) == 0) {
      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {

        //digitalWrite(12, HIGH);
        tlc.setPWM(LEDpin, maxx);
      }
      tlc.write();
      delay((int)heartBeatArray[hbeatIndex]);

      for (int LEDpin = 0; LEDpin <= numLights; LEDpin++) {

        // digitalWrite(12, LOW);
        tlc.setPWM(LEDpin, 0);
      }
      tlc.write();


    }

    hbeatIndex++;
    // Serial.println(hbeatIndex);
    prevMillis = millis();
  }

}


void frustration() {
  /*
  Serial.println("color wipe");
  colorWipe(65535, 0, 0, 100); // "Red" (depending on your LED wiring)
  delay(200);
  colorWipe(0, 65535, 0, 100); // "Green" (depending on your LED wiring)
  delay(200);
  colorWipe(0, 0, 65535, 100); // "Blue" (depending on your LED wiring)
  delay(200);
  */

  //Serial.println("randbow");
  //rainbowCycle(5);

  /* basic control
    tlc.setPWM(11, 10000);
    tlc.write();
    delay(1000);
    tlc.setPWM(11, 5000);
    tlc.write();
    delay(1000);
    tlc.setPWM(11, 0);
    tlc.write();
    delay(1000);
  */
  /*
    uint32_t j;
    for (j = 0; j < 65535; j = j + 10) {
      for (int i = 0; i < 12; i++) {
        tlc.setPWM(i, j);
      }
      tlc.write();
    }

    delay(1000);
    for (int i = 0; i < 12; i++) {
      tlc.setPWM(i, 0);
    }
    tlc.write();
    delay(1000);
    */
  /*
    // loop around
    uint32_t j;
    uint32_t limiter = 500;
    uint32_t k;
    // start at one light  // light increase as go around
    for (int i = 0; i < 12; i++) {
      // increase // level increase
      for (j = 0; j < limiter; j++) {
        tlc.setPWM(i, j);
        /*
        for (k = limiter; k > 0; k--) {
          tlc.setPWM(i - 1, k); // turn the former one down
        }

  tlc.write();
  }

  // tlc.write();
  Serial.print("i is: "); Serial.println(i);
  Serial.print("j is: "); Serial.println(j);
  // pass to next light
  //limiter = limiter + 500; // increase
  */

  /*
    uint32_t level_increment = maxx / numLights;  // divide the num of lights by the max, to evenly distribute
    uint32_t currentMax = level_increment + startLevel;  //our limit for each level will gradually increase for each light as we progress
    int timer = 30;
    */
  /*
  for (int c = 0; c < numLights; c++) {  // for every light
    tlc.setPWM(c, startLevel);    // set the light to the startLevel
    tlc.write();
    delay(200);
  */

  /*
    for (int c = 0; c < numLights; c++) { //while (c < numLights) { // for every light
      // if (startLevel < currentMax * 0.5) {
      tlc.setPWM(c, startLevel * 0.2 );
      tlc.write();
      delay(30);
      tlc.setPWM(c + 1, startLevel * 0.5);
      tlc.write();
      delay(30);
      tlc.setPWM(c + 2, startLevel);
      tlc.write();
      delay(30);
      tlc.setPWM(c, 0); // turn the light off
      tlc.write();
      delay(30);
      tlc.setPWM(c + 1, 0); // turn the light off
      tlc.write();
      delay(30 * 2);
      // }
      //tlc.setPWM(numLights - 1, 0); // turn the light off
      //tlc.write();
      //delay(100);

      /*
          if (startLevel >= currentMax * 0.5) {
            for (uint32_t decreaseLevel = startLevel; decreaseLevel > 0; decreaseLevel = decreaseLevel-100) {
              if(decreaseLevel > -1){
              tlc.setPWM(c - 1, decreaseLevel);
              tlc.write();
              }// if
            }
          }
        */


  //Serial.println("In for");

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
    /*
    if (flag1 == 1 && flag2 == 0) {  // pass it to the next LED aka roll over
      tlc.setPWM(c + 1, level_1);  // light the next led with the roll over value
      tlc.write();
      level_1 = level_1 + 100;  // increase the value
    }
    if (level_1 > currentMax * 0.5) { // if it's byeond a 2nd level
      flag2 = 1;
      flag1 = 0;
      level_2 = level_1;
    }
    if (flag2 == 1 && flag1 == 0) {
      tlc.setPWM(c + 2, level_2); // pass to the next LED aka roll over again
      tlc.write();
      level_2 = level_2 + 100;   // increase that level

      for (uint32_t j = startLevel; j >= 0; j = j - 100) { // decrease 1st LED
        if (j > -1) {
          tlc.setPWM(c, j);
          tlc.write();
        }// if
      } // for (decrease)

    } //third led
    */
    startLevel = startLevel + 100;
  }// for loop
  Serial.print("flag1 is: "); Serial.println(flag1);
} // frustration

/*
s = digitalRead(SPARKLE);
u = digitalRead(UP);
h = digitalRead(HEART);
//Serial.print("XX is: "); Serial.println(xx);
// Serial.print("S is: "); Serial.println(s);
// Serial.print("H is: "); Serial.println(h);
Serial.println("PWM");
if (s == LOW || u == LOW || h == LOW) {  // if any of the other buttons are pressed, break
  clearRegisters();
  writeRegisters();
  xx = 1; // so we can exit loop
  pwm_state = 0;  //so we can get back in, if need be
}
*/

/*
void updateShiftRegister() {
  digitalWrite(RCLK_Pin, LOW);
  shiftOut(SER_Pin, SRCLK_Pin, LSBFIRST, leds);
  digitalWrite(RCLK_Pin, HIGH);
}


void setBrightness(byte brightness) { // 0 to 255
  analogWrite(outputEnablePin, 255 - brightness);
}
*/

// Fill the dots one after the other with a color
void colorWipe(uint16_t r, uint16_t g, uint16_t b, uint8_t wait) {
  for (uint16_t i = 0; i < 8 * NUM_TLC59711; i++) {
    tlc.setLED(i, r, g, b);
    tlc.write();
    delay(wait);
  }
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint32_t i, j;

  for (j = 0; j < 65535; j += 10) { // 1 cycle of all colors on wheel
    for (i = 0; i < 4 * NUM_TLC59711; i++) {
      Wheel(i, ((i * 65535 / (4 * NUM_TLC59711)) + j) & 65535);
    }
    tlc.write();
    delay(wait);
  }
}

// Input a value 0 to 4095 to get a color value.
// The colours are a transition r - g - b - back to r.
void Wheel(uint8_t ledn, uint16_t WheelPos) {
  if (WheelPos < 21845) {
    tlc.setLED(ledn, 3 * WheelPos, 65535 - 3 * WheelPos, 0);
  } else if (WheelPos < 43690) {
    WheelPos -= 21845;
    tlc.setLED(ledn, 65535 - 3 * WheelPos, 0, 3 * WheelPos);
  } else {
    WheelPos -= 43690;
    tlc.setLED(ledn, 0, 3 * WheelPos, 65535 - 3 * WheelPos);
  }
}

