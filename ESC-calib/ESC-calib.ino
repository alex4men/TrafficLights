#include <Servo.h>

const byte motorPin = 47;

Servo motor;

int microseconds = 1500;

void setup() {
  Serial.begin(115200);
  motor.attach(motorPin);
}

void loop() {
  if (Serial.available()) {
    microseconds = Serial.parseInt();
    motor.writeMicroseconds(microseconds);
    //debug
    Serial.println(microseconds);
  }
}
