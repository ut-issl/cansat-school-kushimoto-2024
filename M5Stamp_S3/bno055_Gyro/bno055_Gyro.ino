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
  Serial.println("BNO055 test");

  // SDAピンとSCLピンを指定
  Wire.begin(SDA_PIN, SCL_PIN);

  // BNO055センサの初期化
  if (!bno.begin()) {
    // センサの初期化に失敗した場合の処理
    Serial.print("No BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1)
      ;
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
}

void loop() {
  sensors_event_t event;
  bno.getEvent(&event, Adafruit_BNO055::VECTOR_GYROSCOPE);

  // rad/s
  Serial.print("X軸:");
  Serial.print(event.gyro.x);
  Serial.print(",");
  Serial.print("Y軸:");
  Serial.print(event.gyro.y);
  Serial.print(",");
  Serial.print("Z軸:");
  Serial.print(event.gyro.z);
  Serial.print(",btm:-20,top:20");
  Serial.println("");

  delay(100);
}
