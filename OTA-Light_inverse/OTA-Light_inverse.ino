/*************
 * This program is for Makely TrafficLights v1.0,
 * for running them synchroniously in city model
 * and control them from a server.
 *
 * Alex Fomenko info@makely.ru
 * Makely.ru (c) 2017
***************/
//Red, RedYellow, Green, BlinkingGreen, Yellow -> 0, 1, 2, 3, 4
#include <SoftwareSerial.h>
//< OTA code
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

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

SoftwareSerial swSer(14, 12, false, 256);

const int stages = 5;
byte stage = 0;
const int transmitInterval = 33, blinkInterval = 500;
const byte voltagePin = A0, redLed = 4, yellowLed = 0, greenLed = 13;

const int flatBattLevel = 10;

// TODO: Add lastPressTime
unsigned long curTime = 0, lastChangeTime = 0, lastTransmitTime = 0, lastBlinkTime = 0, startTime = 0;
const int stageIntervals[stages] = {7000, 2000, 4000, 3000, 2000}; //Red, RedYellow, Green, BlinkingGreen, Yellow;
const byte stageCmds[stages] =     {0, 1, 2, 3, 4};
const byte startCmd = 83, stopCmd = 79, resetCmd = 82;
boolean isStarted = false, finishedA = false, finishedB = false;

byte curStage = 0;

boolean greenOn = false;

void setup() {
//< OTA code
  Serial.begin(115200);

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

  int bLevel = battery_level();
  if (bLevel < flatBattLevel) {
    for(int i = 0; i < 3; i++){
      digitalWrite(redLed, HIGH);
      delay(500);
      digitalWrite(redLed, LOW);
      delay(500);
    }
    ESP.deepSleep(0);
  }


  Serial.printf("Connecting to %s ", ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED){
    static byte ledState;
    delay(500);
    Serial.print(".");
    ledState = digitalRead(yellowLed);
    digitalWrite(yellowLed, !ledState);
  }
  /* switch off led */
  digitalWrite(yellowLed, LOW);
  digitalWrite(espLed, HIGH);

  Serial.println(" connected");

//< UDP code
  Udp.begin(port);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), port);
//>

  /* configure OTA server events */
  ArduinoOTA.onStart([]() { // switch off all the PWMs during upgrade
                      leds_Test(250);
                  });

  ArduinoOTA.onEnd([]() { // do a fancy thing with our board led at end
                          leds_Test(250);
                        });

   ArduinoOTA.onError([](ota_error_t error) { ESP.restart(); });

   /* setup the OTA server */
   ArduinoOTA.begin();
   Serial.println("Ready");
//>

  swSer.begin(115200);

  curTime = millis();
  lastChangeTime = curTime;
  lastTransmitTime = curTime;
  lastBlinkTime = curTime;

  // TODO: Add lastPressTime
}

void loop() {
  ArduinoOTA.handle(); // OTA Code

  receiveCmdUdp();
  curTime = millis();
  transmit(stageCmds[stage]);
  lightsUp(stageCmds[stage]);
  // TODO: Add switching on button Press
  if ((isStarted) && (millis() - lastChangeTime) > stageIntervals[stage]) {
    lastChangeTime = millis();
    stage = (stage + 1) % stages;
  }
}

void receiveCmdUdp() {
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }

    //debug
    Serial.printf("UDP packet contents: %s\n", incomingPacket);
    switch (incomingPacket[0]) { // default is the 0'th element
      case startCmd: //S Start
        if(!isStarted) {
          isStarted = true;
          startTime = millis();
          lastChangeTime = startTime;
          stage = 0;
        }
        break;
      case stopCmd: //O Stop
        if(isStarted) {
          isStarted = false;
        }
        lastChangeTime = startTime;
        stage = 0;
        break;
      case resetCmd: //R Reset
        if(isStarted) {
          isStarted = false;
        }
        lastChangeTime = startTime;
        stage = 0;
        finishedA = false;
        finishedB = false;
        break;
      default: //Error Unknown command
        Serial.print("E");
        break;
    }

    // send back a reply, to the IP address we got the packet from and standart port
    Udp.beginPacket(Udp.remoteIP(), port);
    Udp.write(replyPacket);
    Udp.endPacket();
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

void leds_Test(int ms) { //ms - time to turn LEDs on for test
  digitalWrite(redLed, HIGH);
  digitalWrite(yellowLed, HIGH);
  digitalWrite(greenLed, HIGH);
  delay(ms);
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
