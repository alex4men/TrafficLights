/*************
 * This program is for Makely LaserGates v1.0,
 * for running them in city model
 * and control them from a server.
 *
 * Alex Fomenko info@makely.ru
 * Makely.ru (c) 2017
***************/
//< OTA code
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "TP-LINK_39D0";
const char* password = "50239948";
const byte espLed = 2; // ESP built-in LED
//>

IPAddress serverIP(192, 168, 0, 255);
int serverUdpPort = 4210;

//< UDP code
WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
char  replyPacket[] = "ACK";  // a reply string to send back
//>

const byte voltagePin = A0, carAPin = 12, carBPin = 14;

const int flatBattLevel = 10;

const byte startCmd = 83, stopCmd = 79, resetCmd = 82, stopA = 65, stopB = 66;
boolean isStarted = false, finishedA = false, finishedB = false;


void setup() {
//< OTA code
  Serial.begin(115200);

  /* switch on onboard led */
  pinMode(espLed, OUTPUT);
  digitalWrite(espLed, LOW);

  //configure button on pin 5 as an input and enable the internal pull-up resistor
  pinMode(5, INPUT_PULLUP);
  pinMode(carAPin, INPUT);
  pinMode(carBPin, INPUT);

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

  while (WiFi.status() != WL_CONNECTED){
    static byte ledState;
    delay(500);
    Serial.print(".");
    ledState = digitalRead(espLed);
    digitalWrite(espLed, !ledState);
  }
  /* switch off led */
  digitalWrite(espLed, HIGH);

  Serial.println(" connected");

//< UDP code
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
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
}

void loop() {
  ArduinoOTA.handle(); // OTA Code

  receiveCmdUdp();
  //debug
  Serial.print("A: ");
  Serial.print(digitalRead(carAPin));
  Serial.print(" B: ");
  Serial.println(digitalRead(carBPin));

  if (isStarted) {
    if (!digitalRead(carAPin)) {
			if (!finishedA){
        sendCmdUdp(stopA);
        finishedA = true;

        //debug
        Serial.println("stopA");
      }
    }
    if (!digitalRead(carBPin)) {
			if (!finishedB) {
        sendCmdUdp(stopB);
				finishedB = true;

        //debug
        Serial.println("stopB");
      }
    }
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
          serverIP = Udp.remoteIP();
          serverUdpPort = Udp.remotePort();
        }
        break;
      case stopCmd: //O Stop
        if(isStarted) {
          isStarted = false;
        }
        break;
      case resetCmd: //R Reset
        if(isStarted) {
          isStarted = false;
        }
        finishedA = false;
        finishedB = false;
        break;
      default: //Error Unknown command
        Serial.print("E");
        break;
    }

    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyPacket);
    Udp.write(digitalRead(carAPin));
    Udp.write(digitalRead(carBPin));
    Udp.endPacket();
  }
}

void leds_Test(int ms) { //ms - time to turn LEDs on for test
  digitalWrite(espLed, LOW);
  delay(ms);
  digitalWrite(espLed, HIGH);
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

void sendCmdUdp(byte message) {
  // send back a reply, to the IP address and port we got the packet from
  Udp.beginPacket(serverIP, serverUdpPort);
  Udp.write(message);
  Udp.endPacket();
}
