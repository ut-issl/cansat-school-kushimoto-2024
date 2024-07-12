#include <Wire.h>

//アドレス指定
#define S11059_ADDR 0x2A
#define CONTROL_MSB 0x00
#define CONTROL_1_LSB 0x84
#define CONTROL_2_LSB 0x04
#define SENSOR_REGISTER 0x03

void setup()
{
  Serial.begin(115200);
  Wire2.begin(); // join i2c bus (address optional for master)

  Wire2.beginTransmission(S11059_ADDR);//I2Cスレーブ「Arduino Uno」のデータ送信開始
  Wire2.write(CONTROL_MSB);//コントロールバイトを指定
  Wire2.write(CONTROL_1_LSB);//ADCリセット、スリープ解除
  Wire2.endTransmission();//I2Cスレーブ「Arduino Uno」のデータ送信終了

  Wire2.beginTransmission(S11059_ADDR);//I2Cスレーブ「Arduino Uno」のデータ送信開始
  Wire2.write(CONTROL_MSB);//コントロールバイトを指定
  Wire2.write(CONTROL_2_LSB);//ADCリセット解除、バスリリース
  Wire2.endTransmission();//I2Cスレーブ「Arduino Uno」のデータ送信終了
}

void loop()
{
  //変数宣言
  int high_byte, low_byte, red, green, blue, IR;

  //delay(1000);//2500msec待機(2.5秒待機)

  Wire2.beginTransmission(S11059_ADDR);//I2Cスレーブ「Arduino Uno」のデータ送信開始
  Wire2.write(SENSOR_REGISTER);//出力データバイトを指定
  Wire2.endTransmission();//I2Cスレーブ「Arduino Uno」のデータ送信終了

  Wire2.requestFrom(S11059_ADDR, 8);//I2Cデバイス「S11059_ADDR」に8Byteのデータ要求
  if(Wire2.available()){
    high_byte = Wire2.read();//high_byteに「赤(上位バイト)」のデータ読み込み
    low_byte = Wire2.read();//high_byteに「赤(下位バイト)」のデータ読み込み
    red = high_byte << 8|low_byte;//1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、redに代入

    high_byte = Wire2.read();//high_byteに「緑(上位バイト)」のデータ読み込み
    low_byte = Wire2.read();//high_byteに「緑(下位バイト)」のデータ読み込み
    green = high_byte << 8|low_byte;//1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、greenに代入

    high_byte = Wire2.read();//high_byteに「青(上位バイト)」のデータ読み込み
    low_byte = Wire2.read();//high_byteに「青(下位バイト)」のデータ読み込み
    blue = high_byte << 8|low_byte;//1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、blueに代入

    high_byte = Wire2.read();//high_byteに「赤外(上位バイト)」のデータ読み込み
    low_byte = Wire2.read();//high_byteに「赤外(下位バイト)」のデータ読み込み
    IR = high_byte << 8|low_byte;//1Byte目のデータを8bit左にシフト、OR演算子で2Byte目のデータを結合して、IRに代入
  }
  int v = Wire2.endTransmission();//I2Cスレーブ「Arduino Uno」のデータ送信終了

  Serial.print(blue);//「blue」をシリアルモニタに送信
  Serial.print(",");//文字列「,」をシリアルモニタに送信
  Serial.print(red);//「red」をシリアルモニタに送信
  Serial.print(",");//文字列「,」をシリアルモニタに送信
  Serial.print(green);//「green」をシリアルモニタに送信
  Serial.print(",");//文字列「,」をシリアルモニタに送信
  Serial.print(IR);//「IR」をシリアルモニタに送信
  Serial.print(",");//文字列「,」をシリアルモニタに送信
  Serial.print(v);
  Serial.println("");//改行
}






