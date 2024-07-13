/***************************************************************************
  This is a library for the BME680 gas, humidity, temperature & pressure sensor

  Designed specifically to work with the Adafruit BME680 Breakout
  ----> http://www.adafruit.com/products/3660

  These sensors use I2C or SPI to communicate, 2 or 4 pins are required
  to interface.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!

  Written by Limor Fried & Kevin Townsend for Adafruit Industries.
  BSD license, all text above must be included in any redistribution
 ***************************************************************************/

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
  while (!Serial)
    ;
  Serial.println(F("BME680 ALL TEST"));

  // SDAピンとSCLピンを指定
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1)
      ;
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
  Serial.print("Temperature = ");
  Serial.print(bme.temperature);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bme.pressure / 100.0);
  Serial.println(" hPa");

  Serial.print("Humidity = ");
  Serial.print(bme.humidity);
  Serial.println(" %");

  //Serial.print("Gas = ");
  //Serial.print(bme.gas_resistance / 1000.0);
  //Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.print("Approx. Altitude1 = ");
  Serial.print(calculateAltitude(bme.pressure, bme.temperature));
  Serial.println(" m");

  Serial.println();
  delay(10);
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