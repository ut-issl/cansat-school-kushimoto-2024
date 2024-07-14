#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

#define SDA_PIN 13
#define SCL_PIN 15

Adafruit_BME680 bme(&Wire);

#define SEALEVELPRESSURE_HPA 1013.25
//#define SEALEVELPRESSURE_HPA 1009.0

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, 44, 43);

  while (!Serial)
    ;
  Serial.println(F("BME680 OpenLog TEST"));


  // SDAピンとSCLピンを指定
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!bme.begin()) {
    while (1) {
      Serial.println("Could not find a valid BME680 sensor, check wiring!");
      Serial2.println("Could not find a valid BME680 sensor, check wiring!");
      delay(1000);
    }
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_2X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_2X);
  //bme.setTemperatureOversampling(BME680_OS_NONE);
  //bme.setHumidityOversampling(BME680_OS_NONE);
  //bme.setPressureOversampling(BME680_OS_NONE);

  //bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_1);

  //ガスセンサをオフにする
  bme.setGasHeater(0, 0);  // 320*C for 150 ms
}

void loop() {
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  Serial.print(millis());
  Serial.print(",");
  Serial.print(bme.temperature);
  Serial.print(",");
  Serial.print(bme.pressure / 100.0);
  Serial.print(",");
  Serial.print(bme.humidity);
  Serial.print(",");
  //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  //Serial.print(",");
  Serial.print(calculateAltitude(bme.pressure, bme.temperature));
  Serial.println();

  Serial2.print(millis());
  Serial2.print(",");
  Serial2.print(bme.temperature);
  Serial2.print(",");
  Serial2.print(bme.pressure / 100.0);
  Serial2.print(",");
  Serial2.print(bme.humidity);
  Serial2.print(",");
  //Serial2.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  //Serial2.print(",");
  Serial2.print(calculateAltitude(bme.pressure, bme.temperature));
  Serial2.println();

  delay(1000);
}

float calculateAltitude(float pressure, float temperature) {
  // 摂氏からケルビンへ変換
  float T = temperature + 273.15;
  // 海面気圧をパスカル単位に変換
  float P0 = SEALEVELPRESSURE_HPA * 100.0;
  // 気温の減率
  float L = 0.0065;
  // 基準温度
  float T0 = 288.15;
  // 気体定数
  float R = 8.3144598;
  // 重力加速度
  float g = 9.80665;
  // 空気のモル質量
  float M = 0.0289644;

  // 高度計算
  return (T / L) * (pow((P0 / pressure), (R * L) / (g * M)) - 1);
}