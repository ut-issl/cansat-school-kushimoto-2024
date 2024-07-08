#define PIN_LED 33
#define MAXBYTE 200  //最大文字数

byte GPSdata[MAXBYTE];
int ReadPoi = 0;              //現在読み込んだ文字数
int WaitMilliseconds = 1000;  //受信待ち時間 1000msec
unsigned long WaitingLimit = 0;
float Ido, Keido;

//***********************
// 0x0aが来るまでSerial1から読み込みます
// WAITMILISECONDSミリ秒間データが来なければタイムアウトします
//***********************
bool ReadGPS() {
  while (Serial1.available() > 0) {
    GPSdata[ReadPoi] = Serial1.read();
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

float convert_to_decimal_degrees(String value) {
  float degree = value.toInt() / 100;
  degree = degree + (value.toFloat() - degree * 100) / 60.0;
  return degree;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  pinMode(PIN_LED, OUTPUT);
  delay(1000);
  Serial.println("GPS Test");
  digitalWrite(PIN_LED, HIGH);
}

void loop() {
  if (Serial.available() > 0) {
    int receivedByte = Serial.read();
    Serial1.write(receivedByte);
  }

  if (ReadGPS()) {
    String gps = String((char *)GPSdata);
    clearPoi();
    //Serial.println(gps);

    if (gps.startsWith("$GPRMC")) {
      Serial.println(gps);
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
        digitalWrite(PIN_LED, HIGH);
      }
    } 
    else {
      digitalWrite(PIN_LED, LOW);
    }
  }
}

