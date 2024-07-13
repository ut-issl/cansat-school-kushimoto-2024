#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>

#define SDA_PIN 13
#define SCL_PIN 15

// BNO055センサのインスタンスを作成
Adafruit_BNO055 bno = Adafruit_BNO055(55);

void setup() {
  // シリアル通信を開始
  Serial.begin(115200);
  //Serial2.begin(19200, SERIAL_8N1, 44, 43);
  delay(1500);
  Serial.println("BNO055 test");

  // SDAピンとSCLピンを指定
  Wire.begin(SDA_PIN, SCL_PIN);

  // BNO055センサの初期化
  if (!bno.begin()) {
    // センサの初期化に失敗した場合の処理
    Serial.print("No BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1) {
      delay(1000);
      Serial.print("No BNO055 detected ... Check your wiring or I2C ADDR!");
    };
  }

  // センサが安定するまで少し待つ
  delay(1000);

  // センサのキャリブレーション状態を確認
  uint8_t system, gyro, accel, mag;
  bno.getCalibration(&system, &gyro, &accel, &mag);
  Serial.print("Calibration values: ");
  Serial.print(system, DEC);
  Serial.print(", ");
  Serial.print(gyro, DEC);
  Serial.print(", ");
  Serial.print(accel, DEC);
  Serial.print(", ");
  Serial.println(mag, DEC);
  delay(50);
}

void loop() {
  sensors_event_t event;
  bno.getEvent(&event, Adafruit_BNO055::VECTOR_LINEARACCEL);

  // m/s^2
  Serial.print("X軸:");
  Serial.print(event.acceleration.x);
  Serial.print(",");
  Serial.print("Y軸:");
  Serial.print(event.acceleration.y);
  Serial.print(",");
  Serial.print("Z軸:");
  Serial.print(event.acceleration.z);
  Serial.print(",btm:-20,top:20");
  Serial.println("");

  //Serial2.print("X軸:");
  //Serial2.print(event.acceleration.x);
  //Serial2.print(",");
  //Serial2.print("Y軸:");
  //Serial2.print(event.acceleration.y);
  //Serial2.print(",");
  //Serial2.print("Z軸:");
  //Serial2.print(event.acceleration.z);
  //Serial2.print(",btm:-20,top:20");
  //Serial2.println("");

  delay(100);
}
