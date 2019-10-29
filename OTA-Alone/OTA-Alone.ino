/*************
 * This program is for Makely TrafficLights v1.0,
 * for running them independently.
 *
 * Alex Fomenko info@makely.ru
 * Makely.ru (c) 2019
***************/
//Red, RedYellow, Green, BlinkingGreen, Yellow -> 0, 1, 2, 3, 4
#include <SoftwareSerial.h>

SoftwareSerial swSer(14, 12, false, 256);

const int stages = 5;
const int transmitInterval = 33, blinkInterval = 500;
const byte voltagePin = A0, redLed = 4, yellowLed = 0, greenLed = 13;
const int flatBattLevel = 30;

unsigned long curTime = 0, lastChangeTime = 0, lastTransmitTime = 0, lastBlinkTime = 0;
const int stageIntervals[stages] = {7000, 2000, 4000, 3000, 2000}; //Red, RedYellow, Green, BlinkingGreen, Yellow;
const byte stageCmds[stages] =     {0, 1, 2, 3, 4};

int buttonPin = 5;

byte curStage = 0;

boolean greenOn = false;
byte inByte;

void setup() {
  Serial.begin(115200);
  swSer.begin(115200);

  //configure button on pin 5 as an input and enable the internal pull-up resistor
  pinMode(buttonPin, INPUT_PULLUP);
  //LED on ESP
  pinMode(2, OUTPUT);

  pinMode(voltagePin, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  leds_Test();
  
  curTime = millis();
  lastChangeTime = curTime;
  lastTransmitTime = curTime;
  lastBlinkTime = curTime;

  int bLevel = battery_level();
  // if (bLevel < flatBattLevel) {
  //   while(1) {

  //   }
  // }
}

void loop() {
  int stage = 0;
  bool isButtonPressed = false;
  for(stage = 0; stage < stages; stage ++){
//    debug
//    Serial.print("Stage: ");
//    Serial.println(stage);
//    
    while(1) {
      
      curTime = millis();
      transmit(stageCmds[stage]);
      lightsUp(stageCmds[stage]);
      // Switching on button Press
      isButtonPressed = !(digitalRead(buttonPin));
//      Serial.print(isButtonPressed);
      
      if (stage == 0 && isButtonPressed) {
        lastChangeTime = curTime;
        break;
      }
      
      if (stage != 0 && (curTime - lastChangeTime) > stageIntervals[stage]) {
        lastChangeTime = curTime;
        break;
      }
      delay(1);
    }
  }
}

void transmit(byte transCmd) {
  if((curTime - lastTransmitTime) > transmitInterval) {
    swSer.write(transCmd);
    lastTransmitTime = curTime;
  }
}

void lightsUp(byte curStage) {
  switch (curStage) {
    case 0: //Red
      digitalWrite(redLed, HIGH);
      digitalWrite(yellowLed, LOW);
      digitalWrite(greenLed, LOW);
      greenOn = false;
      break;
    case 1: //RedYellow
      digitalWrite(redLed, HIGH);
      digitalWrite(yellowLed, HIGH);
      digitalWrite(greenLed, LOW);
      greenOn = false;
      // do something
      break;
    case 2: //Green
      digitalWrite(redLed, LOW);
      digitalWrite(yellowLed, LOW);
      digitalWrite(greenLed, HIGH);
      greenOn = true;
      // do something
      break;
    case 3: //Blinking Green
      // Serial.println("blink");
      if((curTime - lastBlinkTime) > blinkInterval) {
        // Serial.println(curTime);
        digitalWrite(redLed, LOW);
        digitalWrite(yellowLed, LOW);
        if(greenOn) {
          digitalWrite(greenLed, LOW);
          greenOn = false;
        }
        else {
          digitalWrite(greenLed, HIGH);
          greenOn = true;
        }
        lastBlinkTime = curTime;
      }
      break;
    case 4: //Yellow
      digitalWrite(redLed, LOW);
      digitalWrite(yellowLed, HIGH);
      digitalWrite(greenLed, LOW);
      greenOn = false;
      // do something
      break;
  }
}

void leds_Test() {
  digitalWrite(redLed, HIGH);
  digitalWrite(yellowLed, HIGH);
  digitalWrite(greenLed, HIGH);
  delay(1000);
  digitalWrite(redLed, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(greenLed, LOW);
}

int battery_level() {
 
  // read the battery level from the ESP8266 analog in pin.
  // analog read level is 10 bit 0-1023 (0V-1V).
  // our 1M & 220K voltage divider takes the max
  // lipo value of 4.2V and drops it to 0.758V max.
  // this means our min analog read value should be 580 (3.14V)
  // and the max analog read value should be 774 (4.2V).
  int level = analogRead(A0);
 
  // convert battery level to percent
  level = map(level, 580, 774, 0, 100);
  Serial.print("Battery level: "); Serial.print(level); Serial.println("%");
  return level;
}
