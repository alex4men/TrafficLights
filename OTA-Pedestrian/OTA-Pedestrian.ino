/*************
 * This program is for Makely Pedestrian v1.0.
 *
 * Alex Fomenko info@makely.ru
 * Makely.ru (c) 2019
***************/
//Red, RedYellow, Green, BlinkingGreen, Yellow -> 0, 1, 2, 3, 4
#include <SoftwareSerial.h>
//< OTA code
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Servo.h>

Servo myservo;  // create servo object to control a servo
const unsigned int shlackbaum_delay = 3000; //milliseconds
const int analogInPin = A0;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 13; // Analog output pin that the LED is attached to
static unsigned int shlackbaum_state;
int dV, oldSensVal = 0, sensVal = 0;        // value read from the pot
int outVal = 90;        // value output to the servo (analog out)
int threshold = 50;
unsigned int lastShlackTime = 0;


const char* ssid = "Robotraffic-city";
const char* password = "Robotraffic-city";
const byte espLed = 2; // ESP built-in LED
//>

WiFiEventHandler gotIpEventHandler, disconnectedEventHandler;

//< UDP code
WiFiUDP Udp;
unsigned int port = 4210;  // local port to listen on and to reply
char incomingPacket[255];  // buffer for incoming packets
char  replyPacket[] = "ACK";  // a reply string to send back
//>

SoftwareSerial swSer(14, 12);

const int stages = 1;
const int transmitInterval = 70, blinkInterval = 500;
const byte voltagePin = A0, redLed = 4, yellowLed = 0, greenLed = 13;

const int flatBattLevel = 30;

// TODO: Add lastPressTime
unsigned long curTime = 0, lastChangeTime = 0, lastTransmitTime = 0, lastBlinkTime = 0;
const byte stageCmds[stages] =     {5}; // 6 - для стоп сигнала, 5 - для пешеходного перехода

byte curStage = 0;

boolean greenOn = false;
byte inByte;

void setup() {
//< OTA code
  Serial.begin(115200);
  myservo.attach(13);
  shlackbaum_state = 1;
  myservo.write(outVal); 
  delay(1000); 

  /* switch on onboard led */
  pinMode(espLed, OUTPUT);
  digitalWrite(espLed, LOW);

  //configure button on pin 5 as an input and enable the internal pull-up resistor
  pinMode(5, INPUT_PULLUP);

  pinMode(voltagePin, INPUT);
  pinMode(redLed, OUTPUT);
  pinMode(yellowLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  leds_Test(250);

  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  gotIpEventHandler = WiFi.onStationModeGotIP([](const WiFiEventStationModeGotIP& event)
  {
    Serial.print("Station connected, IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(espLed, HIGH);

    //< UDP code
    Udp.begin(port);
    Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), port);
    //>

    /* setup the OTA server */
    ArduinoOTA.begin();
    Serial.println("OTA flashing Ready");
  });

  disconnectedEventHandler = WiFi.onStationModeDisconnected([](const WiFiEventStationModeDisconnected& event)
  {
    Serial.println("Station disconnected");
  });

  /* configure OTA server events */
  ArduinoOTA.onStart([]() { // switch off all the PWMs during upgrade
                      leds_Test(250);
                  });

  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
                          leds_Test(250);
                        });

  ArduinoOTA.onError([](ota_error_t error) { ESP.restart(); });



  swSer.begin(115200);

  curTime = millis();
  lastChangeTime = curTime;
  lastTransmitTime = curTime;
  lastBlinkTime = curTime;

}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
    ArduinoOTA.handle(); // OTA Code
  }

  int stage = 0;
  curTime = millis();
  transmit(stageCmds[stage]);
  lightsUp(stageCmds[stage]);

  oldSensVal = sensVal;
  sensVal = analogRead(analogInPin);
  dV = sensVal - oldSensVal;

//   Serial.print("sensor = ");
//   Serial.print(sensVal);
//   Serial.print("\t dV = ");
//   Serial.println(dV);

//  analogWrite(analogOutPin, outVal);
  
    // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  // delay(17);
  
  switch (shlackbaum_state)
  {
    case 1:
      

      if (dV > threshold) { // enter next shlackbaum_state and save current time
        shlackbaum_state = 2;
        digitalWrite(13, !digitalRead(13));
        lastShlackTime = millis();
        outVal = 180;
        myservo.write(outVal);
        delay(500);
      }


      break;
    case 2:
      if (curTime - lastShlackTime > shlackbaum_delay) { //check if time has passed
        shlackbaum_state = 1;
        digitalWrite(13, !digitalRead(13));
        outVal = 80;
        
        myservo.write(outVal);
        delay(500);
      }
      break;
  }

  delay(17);
}

void transmit(byte transCmd) {
  if((curTime - lastTransmitTime) > transmitInterval) {
    swSer.write(transCmd);
    lastTransmitTime = curTime;
  }
}

void lightsUp(byte curStage) {
  switch (curStage) {
    case 6: //Stop
      digitalWrite(redLed, HIGH);
      digitalWrite(yellowLed, HIGH);
      digitalWrite(greenLed, HIGH);
      break;
    case 5: //Pedestrian
      digitalWrite(redLed, HIGH);
      digitalWrite(yellowLed, LOW);
      digitalWrite(greenLed, HIGH);
      break;
  }
}

void leds_Test(int ms) { //ms - time to turn LEDs on for test
  digitalWrite(redLed, HIGH);
  digitalWrite(yellowLed, HIGH);
  digitalWrite(greenLed, HIGH);
  delay(ms);
  digitalWrite(redLed, LOW);
  digitalWrite(yellowLed, LOW);
  digitalWrite(greenLed, LOW);
  delay(ms);
}
