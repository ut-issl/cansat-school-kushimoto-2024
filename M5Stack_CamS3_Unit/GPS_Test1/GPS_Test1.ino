#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
// #include <esp_bt.h>
// #include <esp_bt_main.h>

#define GPIO_LED 14
#define MAXBYTE 200  //最大文字数

byte GPSdata[MAXBYTE];
int ReadPoi = 0;              //現在読み込んだ文字数
int WaitMilliseconds = 1000;  //受信待ち時間 1000msec
unsigned long WaitingLimit = 0;

float Ido, Keido;

void setup() {
  Serial.begin(115200);
  //Serial.setRxBufferSize(512);  //default 128
  Serial2.begin(9600, SERIAL_8N1, 44, 43);  //G1->RXD, G3->TXD
  //Serial1.setRxBufferSize(512); //default 128
  delay(1000);
  Serial.println("GPS Test");

  // 現在のCPU周波数を表示
  int cpuClock = getCpuFrequencyMhz();
  Serial.print("CPU Frequency: ");
  Serial.print(cpuClock);
  Serial.println(" MHz");
  // if (cpuClock >= 80) {
  //   // CPU周波数を変更
  //   setCpuFrequencyMhz(40);
  //   //10秒待機
  //   delay(60000);
  //   // CPU周波数を変更
  //   setCpuFrequencyMhz(80);
  //   cpuClock = getCpuFrequencyMhz();
  //   Serial.print("CPU Frequency: ");
  //   Serial.print(cpuClock);
  //   Serial.println(" MHz");
  // }

  // WiFiの無効化
  WiFi.mode(WIFI_OFF);
  esp_wifi_stop();
  esp_wifi_deinit();  // 完全にWiFiを無効化する
  Serial.println("WiFiを無効化しました");

  // // Bluetoothの無効化
  // esp_bluedroid_disable();      // Bluedroidを無効化する
  // esp_bluedroid_deinit();       // Bluedroidを初期化解除する
  // esp_bt_controller_disable();  // BTコントローラを無効化する
  // esp_bt_controller_deinit();   // BTコントローラを初期化解除する
  // Serial.println("Bluetoothを無効化しました");

  //青色LEDをOUTPUTに設定
  pinMode(GPIO_LED, OUTPUT);
  //LED消去
  digitalWrite(GPIO_LED, HIGH);

  //GPS_Init();
  //GPS_Select();
}

void loop() {
  if (Serial.available() > 0) {
    int receivedByte = Serial.read();
    Serial2.write(receivedByte);
  }

  if (ReadGPS()) {
    String gps = String((char *)GPSdata);
    clearPoi();
    //Serial.println(gps);

    if (gps.startsWith("$GPRMC")) {
      //Serial.println(gps);
      String values[10];  // 10個までの要素を保存できるようにする
      int numValues = splitString(gps, ',', values, 10);
      float ido = convert_to_decimal_degrees(values[3]);
      float keido = convert_to_decimal_degrees(values[5]);

      Serial.print(ido, 4);
      Serial.print(", ");
      Serial.println(keido, 4);  // 4桁の小数点以下を表示
      if (ido != 36 && keido != 136 && ido != 0 && keido != 0) {
        Ido = ido;
        Keido = keido;

        //LED
        digitalWrite(GPIO_LED, LOW);
        delay(250);
      }
    } else {
      //LED消去
      digitalWrite(GPIO_LED, HIGH);
    }
  }
}

float convert_to_decimal_degrees(String value) {
  float degree = value.toInt() / 100;
  degree = degree + (value.toFloat() - degree * 100) / 60.0;
  return degree;
}

// カンマ区切りのStringを分割する関数
int splitString(String data, char delimiter, String *values, int maxValues) {
  int index = 0;
  int start = 0;
  int end = data.indexOf(delimiter);

  while (end >= 0 && index < maxValues) {
    values[index++] = data.substring(start, end);
    start = end + 1;
    end = data.indexOf(delimiter, start);
  }

  if (index < maxValues) {
    values[index++] = data.substring(start);
  }

  return index;
}

void GPS_Init() {
  Serial2.println("$PSRF100,1,19200,8,1,0*38");
  delay(100);  // 送信後の待機時間

  // Serial1を19200bpsに変更
  Serial2.end();                             // 一旦終了
  Serial2.begin(19200, SERIAL_8N1, 44, 43);  //G1->RXD, G3->TXD
}

//***********************************************
// GGAメッセージ無効化
// $PSRF103,0,0,0,0*25
// GLLメッセージ無効化
// $PSRF103,1,0,0,0*24
// GSAメッセージ無効化
// $PSRF103,2,0,0,0*27
// GSVメッセージ無効化
// $PSRF103,3,0,0,0*26
// VTGメッセージ無効化
// $PSRF103,5,0,0,0*20
// ZDAメッセージ無効化
// $PSRF103,8,0,0,1*2C
//
// RMCメッセージ有効化
// $PSRF103,4,0,1,1*21
//***********************************************
void GPS_Select() {
  bool ok_flag;
  String gps;
  //GGAメッセージ無効化
  ok_flag = false;
  for (int i = 0; i < 4; i++) {
    Serial1.println("$PSRF103,0,0,0,0*25");
    unsigned long waiting = millis() + 5000;
    while (waiting > millis()) {
      if (ReadGPS()) {
        gps = String((char *)GPSdata);
        //Serial.println(gps);
        clearPoi();
        if (gps.startsWith("$Ack ")) {
          ok_flag = true;
          break;
        }
      }
    }
    if (ok_flag) {
      Serial.println("GGAメッセージ無効化");
      break;
    }
  }

  //GLLメッセージ無効化
  ok_flag = false;
  for (int i = 0; i < 4; i++) {
    Serial1.println("$PSRF103,1,0,0,0*24");
    unsigned long waiting = millis() + 5000;
    while (waiting > millis()) {
      if (ReadGPS()) {
        gps = String((char *)GPSdata);
        //Serial.println(gps);
        clearPoi();
        if (gps.startsWith("$Ack ")) {
          ok_flag = true;
          break;
        }
      }
    }
    if (ok_flag) {
      Serial.println("GLLメッセージ無効化");
      break;
    }
  }
  //GSAメッセージ無効化
  ok_flag = false;
  for (int i = 0; i < 4; i++) {
    Serial1.println("$PSRF103,2,0,0,0*27");
    unsigned long waiting = millis() + 5000;
    while (waiting > millis()) {
      if (ReadGPS()) {
        gps = String((char *)GPSdata);
        //Serial.println(gps);
        clearPoi();
        if (gps.startsWith("$Ack ")) {
          ok_flag = true;
          break;
        }
      }
    }
    if (ok_flag) {
      Serial.println("GSAメッセージ無効化");
      break;
    }
  }
  //GSVメッセージ無効化
  ok_flag = false;
  for (int i = 0; i < 4; i++) {
    Serial1.println("$PSRF103,3,0,0,0*26");
    unsigned long waiting = millis() + 5000;
    while (waiting > millis()) {
      if (ReadGPS()) {
        gps = String((char *)GPSdata);
        //Serial.println(gps);
        clearPoi();
        if (gps.startsWith("$Ack ")) {
          ok_flag = true;
          break;
        }
      }
    }
    if (ok_flag) {
      Serial.println("GSVメッセージ無効化");
      break;
    }
  }
  //VTGメッセージ無効化
  ok_flag = false;
  for (int i = 0; i < 4; i++) {
    Serial1.println("$PSRF103,5,0,0,0*20");
    unsigned long waiting = millis() + 5000;
    while (waiting > millis()) {
      if (ReadGPS()) {
        gps = String((char *)GPSdata);
        //Serial.println(gps);
        clearPoi();
        if (gps.startsWith("$Ack ")) {
          ok_flag = true;
          break;
        }
      }
    }
    if (ok_flag) {
      Serial.println("VTGメッセージ無効化");
      break;
    }
  }
  //ZDAメッセージ無効化
  ok_flag = false;
  for (int i = 0; i < 4; i++) {
    Serial1.println("$PSRF103,8,0,0,1*2C");
    unsigned long waiting = millis() + 5000;
    while (waiting > millis()) {
      if (ReadGPS()) {
        gps = String((char *)GPSdata);
        //Serial.println(gps);
        clearPoi();
        if (gps.startsWith("$Ack ")) {
          ok_flag = true;
          break;
        }
      }
    }
    if (ok_flag) {
      Serial.println("ZDAメッセージ無効化");
      break;
    }
  }

  //RMCメッセージ有効化
  ok_flag = false;
  for (int i = 0; i < 4; i++) {
    Serial1.println("$PSRF103,4,0,1,1*21");
    unsigned long waiting = millis() + 5000;
    while (waiting > millis()) {
      if (ReadGPS()) {
        gps = String((char *)GPSdata);
        //Serial.println(gps);
        clearPoi();
        if (gps.startsWith("$Ack ")) {
          ok_flag = true;
          break;
        }
      }
    }
    if (ok_flag) {
      Serial.println("RMCメッセージ有効化");
      break;
    }
  }
}

//***********************
// 0x0aが来るまでSerial1から読み込みます
// WAITMILISECONDSミリ秒間データが来なければタイムアウトします
//***********************
bool ReadGPS() {
  while (Serial2.available() > 0) {
    GPSdata[ReadPoi] = Serial2.read();
    ReadPoi++;

    if (GPSdata[ReadPoi - 1] == 0x0a) {
      GPSdata[ReadPoi - 1] = 0;
      WaitingLimit = 0;
      return true;
    }

    if (GPSdata[ReadPoi - 1] == 0x0d) {
      ReadPoi--;
    }

    if (ReadPoi >= MAXBYTE) {
      clearPoi();
      break;
    }

    WaitingLimit = millis() + WaitMilliseconds;
  }

  if (WaitingLimit == 0) {
    return false;
  }

  if (WaitingLimit < millis()) {
    clearPoi();
    Serial.println("\nTimeOver");
  }
  return false;
}

//***********************
//ReadGPSデータをリセットします
//***********************
void clearPoi() {
  ReadPoi = 0;
  GPSdata[0] = 0;
  WaitingLimit = 0;
}