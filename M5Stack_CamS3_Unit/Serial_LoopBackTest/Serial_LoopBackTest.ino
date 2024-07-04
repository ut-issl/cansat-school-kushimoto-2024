#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 44, 43);

  delay(1500);
  Serial.println("Loop Back Test");
}

void loop() {
  if (Serial.available() > 0) {
    int receivedByte = Serial.read();
    Serial2.write(receivedByte);
    //Serial.write(receivedByte);
  }

  if (Serial2.available() > 0) {
    int receivedByte = Serial2.read();
    Serial.write(receivedByte);
  }
  delay(10);
}