#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <Adafruit_BME680.h>
#include <S11059.h>
#include <utility/imumaths.h>

#define SDA_PIN 13
#define SCL_PIN 15

// Check I2C device address and correct line below (by default address is 0x29 or 0x28)
//                                   id, address
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28, &Wire);  //0x28をわざわざ明記した例
Adafruit_BME680 bme(&Wire);
S11059 colorSensor;

//#define SEALEVELPRESSURE_HPA 1013.25
#define SEALEVELPRESSURE_HPA 1009.0
#define DELAY_MS 100

bool IsBME680_Reading = false;
float Altitude = 0;
unsigned long BME680_EndTime = 0;

void setup(void) {
  Serial.begin(115200);
  while (!Serial) delay(10);  // wait for serial port to open!

  // SDAピンとSCLピンを指定
  Wire.begin(SDA_PIN, SCL_PIN);

  if (!bno.begin()) {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while (1)
      ;
  }

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1)
      ;
  }
  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_1X);
  bme.setHumidityOversampling(BME680_OS_NONE);
  bme.setPressureOversampling(BME680_OS_1X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_0);
  //ガスセンサをオフにする
  bme.setGasHeater(0, 0);  // 320*C for 150 ms

  // 積分モードを固定時間モードに設定
  colorSensor.setMode(S11059_MODE_FIXED);

  // ゲイン選択
  // * S11059_GAIN_HIGH: Highゲイン
  // * S11059_GAIN_LOW: Lowゲイン
  colorSensor.setGain(S11059_GAIN_HIGH);

  // 1色あたりの積分時間設定(下記は指定可能な定数ごとの固定時間モードの場合の積分時間)
  // * S11059_TINT0: 87.5 us
  // * S11059_TINT1: 1.4 ms
  // * S11059_TINT2: 22.4 ms
  // * S11059_TINT3: 179.2 ms
  colorSensor.setTint(S11059_TINT1);

  // ADCリセット、スリープ解除
  if (!colorSensor.reset()) {
    Serial.println("reset failed");
  }

  // ADCリセット解除、バスリリース
  if (!colorSensor.start()) {
    Serial.println("start failed");
  }
  delay(1000);
  Serial.println("All of Sensors Test");
}

void loop(void) {
  sensors_event_t angVelocityData, linearAccelData;
  bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
  bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);

  // m/s^2
  Serial.print("ac-X:");
  Serial.print(linearAccelData.acceleration.x);
  Serial.print(",");
  Serial.print("ac-Y:");
  Serial.print(linearAccelData.acceleration.y);
  Serial.print(",");
  Serial.print("ac-Z:");
  Serial.print(linearAccelData.acceleration.z);

  Serial.print(",");
  Serial.print("gy-X:");
  Serial.print(angVelocityData.gyro.x);
  Serial.print(",");
  Serial.print("gy-Y:");
  Serial.print(angVelocityData.gyro.y);
  Serial.print(",");
  Serial.print("gy-Z:");
  Serial.print(angVelocityData.gyro.z);

  //if (!bme.performReading()) {
  //  Serial.println("Failed to perform reading :(");
  //  return;
  //}
  if (!IsBME680_Reading) {
    if (bme.beginReading()) {
      IsBME680_Reading = true;
      BME680_EndTime = millis() + 500;
    }
  }

  if (IsBME680_Reading) {
    if (BME680_EndTime < millis()) {
      if (bme.endReading()) {
        IsBME680_Reading = false;
        Altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
        //Serial.print(",");
        //Serial.print("Temp:");
        //Serial.print(bme.temperature);
        //Serial.print(",");
        //Serial.print("Press:");
        //Serial.print(bme.pressure / 100.0);
        //Serial.print(",");
        //Serial.print("Hum:");
        //Serial.print(bme.humidity);
        //Serial.print(",");
        //Serial.print("Alt:");
        //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
        //Serial.print(",");
        //Serial.print("Alt1:");
        //Serial.print(calculateAltitude(bme.pressure, bme.temperature));
      }
    }
  }

  Serial.print(",");
  Serial.print("Alt:");
  Serial.print(Altitude);

  // 積分時間よりも長く待機
  // 固定時間モード時のS11059.delay()で実行される待機処理の時間は
  // S11059.setTint(tint)で設定した値に応じて異なります
  colorSensor.delay();

  // センサのデータ用レジスタ(赤、緑、青、赤外)をまとめてリード
  if (colorSensor.update()) {
    //Serial.print(",");
    //Serial.print("Blue:");
    //Serial.print(colorSensor.getBlue());
    //Serial.print(",");
    //Serial.print("Red:");
    //Serial.print(colorSensor.getRed());
    Serial.print(",");
    Serial.print("Green:");
    Serial.print(colorSensor.getGreen());
    //Serial.print(",");
    //Serial.print("IR:");
    //Serial.print(colorSensor.getIR());
  } else {
    Serial.print("");
  }

  Serial.println("");

  delay(DELAY_MS);
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
