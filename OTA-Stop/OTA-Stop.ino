/*************
 * This program is for Makely Stop Sign v1.0.
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

SoftwareSerial swSer(14, 12);

const int stages = 1;
const int transmitInterval = 70, blinkInterval = 500;
const byte voltagePin = A0, redLed = 4, yellowLed = 0, greenLed = 13;

const int flatBattLevel = 30;

// TODO: Add lastPressTime
unsigned long curTime = 0, lastChangeTime = 0, lastTransmitTime = 0, lastBlinkTime = 0;
const byte stageCmds[stages] =     {6}; // 6 - для стоп сигнала, 5 - для пешеходного перехода

byte curStage = 0;

boolean greenOn = false;
byte inByte;

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
      digitalWrite(espLed, HIGH);
      delay(500);
      digitalWrite(espLed, LOW);
      delay(500);
    }
    ESP.deepSleep(0);
  }


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

  // TODO: Add lastPressTime
}

void loop() {
  if(WiFi.status() == WL_CONNECTED){
    ArduinoOTA.handle(); // OTA Code
  }
  int stage = 0;
  curTime = millis();
  transmit(stageCmds[stage]);
  lightsUp(stageCmds[stage]);
  delay(1);
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
