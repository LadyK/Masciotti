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
#define UP 7
#define HEART 4
#define PWM 3
#define OFF 8

int thrown = 0;
bool s;
bool u;
bool h;
bool pwm;
bool s_state;
bool u_state;
bool h_state;
bool pwm_state;
bool xx = false; // this is what helps us break out of loops

long heartBeatArray[] = {
  50, 100, 15, 1200
};

int hbeatIndex = 1;   // this initialization is important or it starts on the "wrong foot"

long prevMillis;
byte leds = 0;

int numLights = 12;  //Number of lights we are controlling

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

boolean registers[numOfRegisterPins];

uint32_t maxx = 65535;
uint32_t startLevel = 0;

void setup() {
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLK_Pin, OUTPUT);
  // pinMode(12, OUTPUT);
  Serial.begin(9600);
  pinMode(SPARKLE, INPUT_PULLUP);
  pinMode(UP, INPUT_PULLUP);
  pinMode(HEART, INPUT_PULLUP);
  pinMode(PWM, INPUT_PULLUP);

  pinMode(outputEnablePin, OUTPUT);
  analogWrite(outputEnablePin, 0);  // set this to ground for digital output

  //reset all register pins
  clearRegisters();
  writeRegisters();
  //Serial.println(numOfRegisterPins);
  pinMode(10, OUTPUT);
  tlc.begin();
  tlc.write();
}


void loop() {
  // sparkle(100);
  //heartBeat(1.0);

  if (digitalRead(SPARKLE) == LOW) {
    s_state = 1;
  }
  else if (digitalRead(HEART) == LOW) {
    h_state = 1;
  }
  else if (digitalRead(UP) == LOW) {
    u_state = 1;
  }

  else if (digitalRead(PWM) == LOW) {
    pwm_state = 1;
  }
  else if (digitalRead(OFF) == LOW) {  // off button
    for (int c = 0; c < 12; c++) {
      tlc.setPWM(c, 0);
      tlc.write();
    }
  }

  else {
    for (int i = 1; i <= 9; i++) {

      setRegisterPin(i, LOW);
    }
    writeRegisters();
  }
  Serial.println(h_state);
  Serial.println(u_state);

  while (s_state == 1 && xx == 0) {  // sparkle
    sparkle_(100, maxx);
    sparkle_(100, maxx / 2); //maxx = 65535
    u = digitalRead(UP);
    h = digitalRead(HEART);
    pwm = digitalRead(PWM);
    //Serial.print("XX is: "); Serial.println(xx);
    //Serial.print("U is: "); Serial.println(u);
    //Serial.print("H is: "); Serial.println(h);
    if (u == LOW || h == LOW || pwm == LOW) {  // if any of the other buttons are pressed, break
      clearRegisters();
      writeRegisters();
      xx = 1; // so we can exit loop
      s_state = 0;  //so we can get back in, if need be

      //if (u == LOW) u_state = 0; no longer necessary
      //if (h == LOW) h_state = 0;
    }
  }

  xx = 0; // flip back

  while (u_state == 1 && xx == 0) {
    ascend(100);
    s = digitalRead(SPARKLE);
    h = digitalRead(HEART);
    pwm = digitalRead(PWM);
    //Serial.print("XX is: "); Serial.println(xx);
    // Serial.print("S is: "); Serial.println(s);
    // Serial.print("H is: "); Serial.println(h);
    Serial.println("ascention");
    if (s == LOW || h == LOW || pwm == LOW) {  // if any of the other buttons are pressed, break
      clearRegisters();
      writeRegisters();
      xx = 1; // so we can exit loop
      u_state = 0;  //so we can get back in, if need be
    }
  }

  xx = 0;



  while (h_state == 1 && xx == 0) {
    heartBeat(1.0);
    s = digitalRead(SPARKLE);
    u = digitalRead(UP);
    pwm = digitalRead(PWM);
    //Serial.print("XX is: "); Serial.println(xx);
    // Serial.print("S is: "); Serial.println(s);
    // Serial.print("H is: "); Serial.println(h);
    Serial.println("heartbeat");
    if (s == LOW || u == LOW || pwm == LOW) {  // if any of the other buttons are pressed, break
      clearRegisters();
      writeRegisters();
      xx = 1; // so we can exit loop
      h_state = 0;  //so we can get back in, if need be
    }
  }

  xx = 0;

  while (pwm_state == 1 && xx == 0) {
    frustration();

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
  }
  analogWrite(outputEnablePin, 0);  // set this to ground for digital output

  xx = 0;




  delay(250);

  /*
  Serial.print("XX is: "); Serial.println(xx);
  Serial.print("S is: "); Serial.println(s);
  Serial.print("U is: "); Serial.println(u);
  Serial.print("H is: "); Serial.println(h);
  */

  /*
    bool newState = digitalRead(BUTTON_PIN); // take a reading
    Serial.println(newState);
    Serial.print("ShowType is  ");
    Serial.println(showType);

    if (newState == LOW && oldState == HIGH) {  // if we are different from the former state
      // debounce
      delay(20);
      // check if we are really on
      newState = digitalRead(BUTTON_PIN); // check again to make sure we are engaged
      if (newState == 0) {                // if so, increase the showType
        showType++;
        if (showType > 3) showType = 0 ;
        startShow(showType);
      }
     // else{
     //   startShow(showType);
     // }

    }
    oldState = newState; // the new state is now the old
  */
  //writeRegisters();  //MUST BE CALLED TO DISPLAY CHANGES
  //Only call once after the values are set how you need.

  //*/

}




//set all register pins to LOW
void clearRegisters() {
  for (int i = numOfRegisterPins - 2; i >=  1; i--) {
    registers[i] = LOW;
  }
}


//Set and display registers
//Only call AFTER all values are set how you would like (slow otherwise)
void writeRegisters() {

  digitalWrite(RCLK_Pin, LOW);  // prepping for the new values

  for (int i = numOfRegisterPins - 2; i >=  1; i--) {
    digitalWrite(SRCLK_Pin, LOW); // The clock pin needs to receive eight pulses
    // the time of each pulse:

    int val = registers[i];  // pull out the new setting for this pin
    // if the data pin is high,
    //then a 1 gets pushed into the shift register,
    //otherwise a 0.


    digitalWrite(SER_Pin, val); // write that value to that pin
    // Input for the next pin that gets shifted in.

    digitalWrite(SRCLK_Pin, HIGH);  // When this pin is pulled high, it will shift the register.



  }
  //When all eight pulses have been received...
  //then enabling the 'Latch' pin
  //copies those eight values to the latch register.
  digitalWrite(RCLK_Pin, HIGH);  //must pull high to send out new values

}

//set an individual pin HIGH or LOW
void setRegisterPin(int index, int value) {
  registers[index] = value;  // store these values for the associated pin
}


/******* Neural Modes ******/
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

void ascend(int d) {
  setRegisterPin(4, HIGH);
  writeRegisters();
  delay(100);
  setRegisterPin(3, HIGH);
  writeRegisters();
  delay(100);
  setRegisterPin(5, HIGH);
  writeRegisters();
  delay(100);
  setRegisterPin(7, HIGH);
  writeRegisters();
  delay(100);

  setRegisterPin(6, HIGH);
  writeRegisters();
  delay(100);
  setRegisterPin(4, LOW);
  writeRegisters();
  delay(100);
  setRegisterPin(9, HIGH);
  writeRegisters();
  delay(100);
  setRegisterPin(3, LOW);
  writeRegisters();
  delay(100);
  setRegisterPin(5, LOW);
  writeRegisters();
  delay(100);
  setRegisterPin(1, HIGH);
  writeRegisters();
  delay(100);
  setRegisterPin(2, HIGH);
  writeRegisters();
  delay(100);
  setRegisterPin(8, HIGH);
  writeRegisters();
  delay(100);
  setRegisterPin(7, LOW);
  writeRegisters();
  delay(100);

  setRegisterPin(6, LOW);
  writeRegisters();
  delay(100);

  setRegisterPin(9, LOW);
  writeRegisters();
  delay(100);

  setRegisterPin(1, LOW);
  writeRegisters();
  delay(100);

  setRegisterPin(2, LOW);
  writeRegisters();
  delay(100);
  setRegisterPin(8, LOW);
  writeRegisters();
  delay(100);

  delay(2000);

}


void heartBeat(float tempo) {
  if ((millis() - prevMillis) > (long)(heartBeatArray[hbeatIndex] * tempo)) {
    hbeatIndex++;
    if (hbeatIndex > 3) hbeatIndex = 0;

    if ((hbeatIndex % 2) == 0) {
      for (int LEDpin = 0; LEDpin <= 8; LEDpin++) {

        //digitalWrite(12, HIGH);
        setRegisterPin(LEDpin, HIGH);
        writeRegisters();
        delay((int)heartBeatArray[hbeatIndex]);
        // digitalWrite(12, LOW);
        setRegisterPin(LEDpin, LOW);
        writeRegisters();


      }
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


  uint32_t level_increment = maxx / numLights;  // divide the num of lights by the max, to evenly distribute
  uint32_t currentMax = level_increment + startLevel;  //our limit for each level will gradually increase for each light as we progress
  int timer = 30;
  /*
  for (int c = 0; c < numLights; c++) {  // for every light
    tlc.setPWM(c, startLevel);    // set the light to the startLevel
    tlc.write();
    delay(200);
  */
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
  /*
        Serial.println("in the while loop");
      } // above half way
      else {
        tlc.setPWM(c, startLevel);    // set the light to the startLevel
        tlc.write();
      }

      // delay(timer);
      startLevel = startLevel + 100;    // increment the startLevel
      //Serial.println("still in while");
    //} // end while

  */
  /*
    for (int c = 0; c < numLights; c++) {  // for every light
      //Serial.print("C is: ");
      //Serial.println(c);
      while ( startLevel < currentMax) {  // while the start level is less than our temporary current
        tlc.setPWM(c, startLevel);    // set the light to the startLevel
        tlc.write();
        //delay(timer);
        if (startLevel > startLevel * .5) { // if the start level is  more than half way
          tlc.setPWM(c + 1, startLevel * .2); // light the next light with half the level
          tlc.write();
          tlc.setPWM(c - 1, startLevel * .05); // light the next light with half the level
          tlc.write();
        }
        // delay(timer);
        startLevel = startLevel + 100;    // increment the startLevel
        //Serial.println("still in while");
      }
  */
  /*
  tlc.setPWM(c, 0); // turn the preceeding light off // snap down right away?
  tlc.write();

  /* not working
  if (startLevel == currentMax - 1) {  // if we are at the max

    while (startLevel > 0) {
      tlc.setPWM(c, startLevel); // turn the preceeding light off // snap down right away?
      tlc.write();
      startLevel = startLevel - 100; // bring the level down
    }
  }

  startLevel = 0;
  Serial.println("out of while");
  Serial.print("C is: ");
  Serial.println(c);

  Serial.println("exit frustration");
}
*/
//currentMax = currentMax + level_increment;

/*

for (int c = 0; c < numLights; c++) {
  tlc.setPWM(c, level);
  tlc.write();
  delay(timer);
  tlc.setPWM(c + 1, maxx / 4);
  tlc.write();
  delay(timer);
  tlc.setPWM(c, 0);
  tlc.write();
  delay(timer * 2);
}

for (int c = numLights - 1; c >= 0; c--) {
  tlc.setPWM(c, maxx / 2);
  tlc.write();
  delay(timer);
  tlc.setPWM(c - 1, maxx / 2);
  tlc.write();
  delay(timer);
  tlc.setPWM(c, 0);
  tlc.write();
  delay(timer * 2);
}


*/




/*
Serial.println("oneFade, up");
uint32_t i;
for ( i = 0; i <= 65535; i = i + 10) {
  tlc.setPWM(7, i); // not sure why it's not working
  tlc.write();
  //delay(100);

}
//delay(1000);

Serial.println("down ");
for ( i < 65535; i > 0; i = i - 10) {
  tlc.setPWM(7, i);
  tlc.write();
  //delay(100);
}


//******* OLD CODE

/*
Serial.println("Begin frustration");
leds = 0;
updateShiftRegister();
//delay(2000);
Serial.println("Up a bit");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 0; b <= 100; b++) { // up a bit
  setBrightness(b);
  delay(100);
}
updateShiftRegister();
delay(2000);


// ***** down + up
updateShiftRegister();
//delay(2000);
Serial.println("Down a bit");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 100; b >= 25; b--) { // down a bit
  setBrightness(b);
  delay(10);
}
updateShiftRegister();



updateShiftRegister();
//delay(2000);
Serial.println("Up a bit");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 25; b <= 100; b++) { // up a bit
  setBrightness(b);
  delay(10);
}
updateShiftRegister();



updateShiftRegister();
//delay(2000);
Serial.println("Down a bit");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 100; b >= 25; b--) { // down a bit
  setBrightness(b);
  delay(10);
}
updateShiftRegister();


updateShiftRegister();
//delay(2000);
Serial.println("Up a bit");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 25; b <= 100; b++) { // up a bit
  setBrightness(b);
  delay(10);
}
updateShiftRegister();



updateShiftRegister();
//delay(2000);
Serial.println("Down a bit");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 100; b >= 25; b--) { // down a bit
  setBrightness(b);
  delay(10);
}
updateShiftRegister();




updateShiftRegister();
//delay(2000);
Serial.println("Up a bit");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 25; b <= 100; b++) { // up a bit
  setBrightness(b);
  delay(10);
}
updateShiftRegister();




updateShiftRegister();
//delay(2000);
Serial.println("Down a bit x");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 100; b >= 25; b--) { // down a bit
  setBrightness(b);
  delay(10);
}
updateShiftRegister();




updateShiftRegister();
//delay(2000);
Serial.println("Up a bit");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 25; b <= 100; b++) { // up a bit
  setBrightness(b);
  delay(10);
}
updateShiftRegister();

delay(4000);


updateShiftRegister();
//delay(2000);
Serial.println("Up a bit more");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

//**** up a bit more
for (byte b = 100; b <= 175; b++) { // up a bit
  setBrightness(b);
  delay(10);
}
updateShiftRegister();
delay(4000);


updateShiftRegister();
//delay(2000);
Serial.println("Up even a bit more");
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}

for (byte b = 175; b < 255; b++) { // up even a bit more
  setBrightness(b);
  delay(50);
}

updateShiftRegister();
//Serial.println("Done");
delay(2000);




Serial.println("Begin Wobble");

updateShiftRegister();
//delay(2000);
for (int i = 0; i < 8; i++) {
  bitSet(leds, i);
  updateShiftRegister();
  //delay(50);
}
Serial.println("down");
// wobble a bit
for (byte b = 255; b > 0; b--) { // down a bit
  setBrightness(b);
  delay(10);
}


updateShiftRegister();
//delay(1000);

for (int j = 0; j < 15; j++) {

  //Serial.println("...");
  updateShiftRegister();
  //delay(2000);
  for (int i = 0; i < 8; i++) {
    bitSet(leds, i);
    updateShiftRegister();
    //delay(50);
  }

  setBrightness(255);
  delay(100);

  updateShiftRegister();
  //delay(2000);
  for (int i = 0; i < 8; i++) {
    bitSet(leds, i);
    updateShiftRegister();
    //delay(50);
  }
  setBrightness(0);
  delay(100);
  updateShiftRegister();
}



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


void updateShiftRegister() {
  digitalWrite(RCLK_Pin, LOW);
  shiftOut(SER_Pin, SRCLK_Pin, LSBFIRST, leds);
  digitalWrite(RCLK_Pin, HIGH);
}


void setBrightness(byte brightness) { // 0 to 255
  analogWrite(outputEnablePin, 255 - brightness);
}

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

