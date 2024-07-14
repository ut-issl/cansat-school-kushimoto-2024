#include "HWCDC.h"
#include "Print.h"
#include <SD.h>
#include <SPI.h>
#include "MakeAVI.h"
#include "camera.h"

void printHexln(uint32_t data) {
  char buffer[9];                 // 8文字+null終端文字用のバッファ
  sprintf(buffer, "%08X", data);  // 16進数に変換してバッファに格納
  Serial.println(buffer);         // バッファの内容をシリアル出力
}
void printHex(uint32_t data) {
  char buffer[9];                 // 8文字+null終端文字用のバッファ
  sprintf(buffer, "%08X", data);  // 16進数に変換してバッファに格納
  Serial.print(buffer);           // バッファの内容をシリアル出力
}

//**********
// 文字列から末尾の数字部分を除去した文字部分を取り出す関数
//**********
String extractPrefix(const String& filename) {
  int dotPos = filename.lastIndexOf('.');
  String baseName = (dotPos == -1) ? filename : filename.substring(0, dotPos);

  int len = baseName.length();
  int pos = len;
  while (pos > 0 && isDigit(baseName.charAt(pos - 1))) {
    pos--;
  }
  return baseName.substring(0, pos);
}

//**********
// 指定されたディレクトリ内の.jpgファイルをリストアップする関数
//**********
String pickupJpgFiles(const String& foldername) {
  String jpgFilename = "";
  File dir = SD.open(foldername);
  if (!dir) {
    Serial.print("フォルダのオープンに失敗しました: ");
    Serial.println(foldername);
    return jpgFilename;
  }

  File file = dir.openNextFile();
  String filename = "";
  while (file) {
    filename = file.name();
    file.close();

    if (filename.endsWith(".jpg")) {
      jpgFilename = filename;
      break;
    }
    file = dir.openNextFile();
  }
  dir.close();
  return jpgFilename;
}

String getBaseJpgname(const String& foldername) {
  String jpgFilename = pickupJpgFiles(foldername);
  return extractPrefix(jpgFilename);
}

void CreateAviFromJpegs(String& foldername, int width, int height, uint32_t maxJpgSize) {
  CreateAviFromJpegs(foldername, FRAME_COUNT, FPS, width, height, maxJpgSize);
}
void CreateAviFromJpegs(String& foldername, uint32_t frameCount, uint32_t fps, int width, int height, uint32_t maxJpgSize) {
  long fileSizePosition;
  long hdrlListSizePosition;
  long strlListSizePosition;
  long endStrlListPosition;
  long endHdrlListPosition;
  long moviListSizePosition;
  long moviDataOffset;
  long currentFrameOffset;
  long endMoviListPosition;
  long idx1SizePosition;
  long endIdx1Position;

  uint32_t frameTime = 1000000 / fps;

  // ヒープ上のメモリ確保
  const size_t bufferSize = 32768;
  uint8_t* buffer = (uint8_t*)malloc(bufferSize);
  if (buffer == NULL) {
    printf("メモリ確保に失敗しました\n");
    return;
  }
  //const size_t bufferSize = 4192;
  //uint8_t buffer[bufferSize];

  //jpgのベース名を求めます
  String baseJpgname = getBaseJpgname(foldername);
  baseJpgname = foldername + "/" + baseJpgname;
  Serial.println(baseJpgname);

  File aviFile = SD.open(foldername + "/temp.avi", FILE_WRITE);
  if (!aviFile) {
    Serial.println("AVIファイルの作成に失敗しました");
    // メモリ解放
    free(buffer);
    return;
  }

  // RIFFヘッダー
  aviFile.write((const uint8_t*)"RIFF", 4);
  fileSizePosition = aviFile.position();
  write4bytes(aviFile, 0);  // ファイルサイズ (後で更新)
  aviFile.write((const uint8_t*)"AVI ", 4);

  // メインヘッダーリスト
  aviFile.write((const uint8_t*)"LIST", 4);
  hdrlListSizePosition = aviFile.position();
  write4bytes(aviFile, 0);  // ファイルサイズ (後で更新)
  aviFile.write((const uint8_t*)"hdrl", 4);

  // AVIメインヘッダー
  aviFile.write((const uint8_t*)"avih", 4);
  write4bytes(aviFile, 56);  // チャンクサイズ 56バイト
  //ここから下が56バイト
  {
    aviFile.write((const uint8_t*)&frameTime, 4);   // フレーム持続時間
    write4bytes(aviFile, 0);                        // 転送レートの最大値
    write4bytes(aviFile, 0);                        // このサイズの倍数にパディングされる
    write4bytes(aviFile, 0x910);                    // フラグ
    aviFile.write((const uint8_t*)&frameCount, 4);  // 全フレーム数
    write4bytes(aviFile, 0);                        // 初期フレーム(無視する)
    write4bytes(aviFile, 1);                        // ストリーム数
    write4bytes(aviFile, 0x100000);                 // サンプル/秒
    aviFile.write((const uint8_t*)&width, 4);       // 横幅
    aviFile.write((const uint8_t*)&height, 4);      // 縦幅
    write4bytes(aviFile, 0);                        // 保留領域
    write4bytes(aviFile, 0);                        // 保留領域
    write4bytes(aviFile, 0);                        // 保留領域
    write4bytes(aviFile, 0);                        // 保留領域
  }

  // ストリームリスト
  aviFile.write((const uint8_t*)"LIST", 4);  //
  strlListSizePosition = aviFile.position();
  write4bytes(aviFile, 0);  // リストチャンクサイズ (後で更新)
  aviFile.write((const uint8_t*)"strl", 4);

  //周期から、周波数を表す値の分子と分母の互いに素の整数を求めます
  uint32_t framePer = 0;
  uint32_t bunboSec = 1;
  {
    framePer = 1000000;  // 1秒をマイクロ秒で表したもの
    bunboSec = frameTime;
    // 最大公約数を求める
    uint32_t gcd = GCD(framePer, bunboSec);
    // 分子と分母を最大公約数で割って簡約化する
    framePer /= gcd;
    bunboSec /= gcd;
  }

  // ストリームヘッダーとフォーマット
  WriteStreamHeader(aviFile, width, height, maxJpgSize, framePer, bunboSec, frameCount);

  // 更新ストリームリストサイズ
  endStrlListPosition = aviFile.position();

  //OpenDML拡張ヘッダー
  OpenDMLHeader(aviFile);

  // 更新ヘッダーリストサイズ
  endHdrlListPosition = aviFile.position();

  //ソフトウエア名称ヘッダー
  WriteSoftwareNameHeader(aviFile);

  // ムービーリスト
  aviFile.write((const uint8_t*)"LIST", 4);  //
  moviListSizePosition = aviFile.position();
  write4bytes(aviFile, 0);  // リストチャンクサイズ (後で更新)
  aviFile.write((const uint8_t*)"movi", 4);

  // 各JPEGファイルを書き込み
  {
    moviDataOffset = aviFile.position();
    currentFrameOffset = aviFile.position();
    uint32_t length;
    uint32_t val = 0;
    size_t bytesRead;

    for (uint32_t i = 0; i < frameCount; i++) {
      String fileName = baseJpgname + String(i) + ".jpg";
      File file = SD.open(fileName.c_str(), FILE_READ);
      if (!file) {
        Serial.println("フレームファイルの読み込みに失敗しました");
        continue;
      }

      // フレームデータを書き込む
      aviFile.write((const uint8_t*)"00dc", 4);  // フレームチャンクのID
      length = file.size();
      //Jpegサイズ書き込み
      aviFile.write((const uint8_t*)&length, 4);
      //Jpeg本体の書き込み
      while ((bytesRead = file.read(buffer, bufferSize)) > 0) {
        aviFile.write(buffer, bytesRead);
      }

      //チャンクサイズが奇数にならないように調整します。
      if (length % 2 == 1) {
        length += 1;
        aviFile.write((const uint8_t*)&val, 1);
      }
      currentFrameOffset += length + 8;  // 8バイトは "00dc" と サイズのための領域
      file.close();
    }
  }

  // 更新ムービーリストサイズ
  endMoviListPosition = aviFile.position();

  // インデックス
  {
    uint32_t length = 0;
    aviFile.write((const uint8_t*)"idx1", 4);
    idx1SizePosition = aviFile.position();
    write4bytes(aviFile, 0);  // インデックスサイズ (後で更新)

    currentFrameOffset = moviDataOffset + 4;

    for (uint32_t i = 0; i < frameCount; i++) {
      String fileName = baseJpgname + String(i) + ".jpg";
      File file = SD.open(fileName.c_str(), FILE_READ);
      if (!file) {
        Serial.println("フレームファイルの読み込みに失敗しました");
        continue;
      }

      length = file.size();
      file.close();

      aviFile.write((const uint8_t*)"00dc", 4);
      write4bytes(aviFile, 0x10);  // フラグ
      write4bytes(aviFile, (uint32_t)(currentFrameOffset - moviDataOffset));
      aviFile.write((const uint8_t*)&length, 4);
      //チャンクlengthは偶数にしているので
      if (length % 2 == 1) {
        length += 1;
      }
      currentFrameOffset += length + 8;
    }

    // 更新インデックスサイズ
    endIdx1Position = aviFile.position();
  }
  aviFile.close();

  //後で更新する部分の更新
  Serial.println("チャンクサイズを更新します");
  {
    // 元ファイルを読み込みモードで開く
    File src = SD.open(foldername + "/temp.avi", FILE_READ);
    if (!src) {
      Serial.println("AVIファイルの作成に失敗しました");
      // メモリ解放
      free(buffer);
      return;
    }

    // 新しいファイルを書き込みモードで開く
    //String aviFilename = foldername + foldername + ".avi";
    aviFile = SD.open(foldername + foldername + ".avi", FILE_WRITE);
    if (!aviFile) {
      Serial.println("AVIファイルの作成に失敗しました");
      src.close();
      // メモリ解放
      free(buffer);
      return;
    }
    long fileSize = src.size();

    long positions[] = { fileSizePosition, hdrlListSizePosition, strlListSizePosition, moviListSizePosition, idx1SizePosition };
    uint32_t sizeValues[] = {
      static_cast<uint32_t>(fileSize - 8),
      static_cast<uint32_t>(endHdrlListPosition - hdrlListSizePosition - 4),
      static_cast<uint32_t>(endStrlListPosition - strlListSizePosition - 4),
      static_cast<uint32_t>(endMoviListPosition - moviListSizePosition - 4),
      static_cast<uint32_t>(endIdx1Position - idx1SizePosition - 4)
    };
    int positionIndex = 0;
    size_t bytesRead;
    size_t bytesWritten = 0;
    size_t bufferIndex = 0;

    while (bytesWritten < fileSize) {
      // 次の特定位置までの残りバイト数を計算
      size_t nextPosition = positionIndex < 5 ? positions[positionIndex] : fileSize;
      size_t bytesToRead = std::min(bufferSize, nextPosition - bytesWritten);

      // バッファサイズを調整して読み込み
      bytesRead = src.read(buffer, bytesToRead);
      aviFile.write(buffer, bytesRead);
      bytesWritten += bytesRead;

      if (bytesRead == 0) break;  // 読み込みが終わった場合

      if (positionIndex < 5 && bytesWritten == positions[positionIndex]) {
        aviFile.write((const uint8_t*)&sizeValues[positionIndex], 4);
        src.read(buffer, 4);
        positionIndex++;
        bytesWritten += 4;
      }
    }
    src.close();
    aviFile.close();
  }

  if (!SD.remove(foldername + "/temp.avi")) {
    Serial.println("テンポラリファイルの削除に失敗しました");
  }

  // メモリ解放
  free(buffer);
}

//**************************************************
// 4バイトのデータにして書き込みます
//**************************************************
void write4bytes(File& aviFile, uint32_t dword) {
  aviFile.write((const uint8_t*)&dword, 4);
}

//**************************************************
// ユークリッドの互除法を使用して最大公約数を求める
//**************************************************
uint32_t GCD(uint32_t a, uint32_t b) {
  uint32_t temp;
  while (b != 0) {
    temp = b;
    b = a % b;
    a = temp;
  }
  return a;
}

//**************************************************
// ストリームヘッダーを書きます
//**************************************************
void WriteStreamHeader(File& aviFile, uint32_t width, uint32_t height, long maxJpegSize, uint32_t framePer, uint32_t bunboSec, uint32_t frameCount) {
  aviFile.write((const uint8_t*)"strh", 4);  //
  write4bytes(aviFile, 56);                  // チャンクサイズ
  //ここから56バイト
  {
    aviFile.write((const uint8_t*)"vids", 4);  // ビデオストリーム
    aviFile.write((const uint8_t*)"MJPG", 4);  // JPEGコーデック
    write4bytes(aviFile, 0);                   // フラグ
    write4bytes(aviFile, 0);                   // WORD wPriority=0 と、WORD wLanguage=0 をの止めて4バイトとして書き込む

    write4bytes(aviFile, 0);  // 初期フレーム dwInitialFrames

    // dwScale とdwRate を使って、1秒に何フレームかを示します
    // 0.5 (frames/second)だとすると、(dwRate=1) / (dwScale=2)=0.5 となります
    aviFile.write((const uint8_t*)&bunboSec, 4);  // dwScale
    aviFile.write((const uint8_t*)&framePer, 4);  // dwRate

    write4bytes(aviFile, 0);                         // dwStart ストリームの開始時間
    aviFile.write((const uint8_t*)&frameCount, 4);   // dwLength
    aviFile.write((const uint8_t*)&maxJpegSize, 4);  // dwSuggestedBufferSize 最大のjpegファイルサイズ

    write4bytes(aviFile, 0xFFFFFFFF);  // dwQuality ストリームの質を示すべきである．重要ではない．
    write4bytes(aviFile, 0);           // ストリームの最小単位（これ以上細分すべきでない単位）のバイト数

    //RECT rcFrame
    uint32_t val = 0;
    aviFile.write((const uint8_t*)&val, 2);     // rcFrame.right
    aviFile.write((const uint8_t*)&val, 2);     // rcFrame.right
    aviFile.write((const uint8_t*)&width, 2);   // rcFrame.right
    aviFile.write((const uint8_t*)&height, 2);  // rcFrame.bottom
  }

  // ストリームフォーマット
  aviFile.write((const uint8_t*)"strf", 4);  // ビデオストリーム
  write4bytes(aviFile, 40);                  // チャンクサイズ
  //ここから40バイトです
  {
    uint32_t val = 0;
    write4bytes(aviFile, 40);                   // ビットマップ情報ヘッダーサイズ
    aviFile.write((const uint8_t*)&width, 4);   // 横幅
    aviFile.write((const uint8_t*)&height, 4);  // 縦幅
    val = 1;
    aviFile.write((const uint8_t*)&val, 2);  // プレーン数
    val = 24;
    aviFile.write((const uint8_t*)&val, 2);    // ビット/ピクセル
    aviFile.write((const uint8_t*)"MJPG", 4);  // 圧縮形式
    val = width * height * 3;
    aviFile.write((const uint8_t*)&val, 4);  // 画像サイズ
    write4bytes(aviFile, 0);                 // Xピクセル/メーター
    write4bytes(aviFile, 0);                 // Yピクセル/メーター
    write4bytes(aviFile, 0);                 // 使用されるカラー数
    write4bytes(aviFile, 0);                 // 重要なカラー数
  }

  // JUNKフォーマット
  aviFile.write((const uint8_t*)"JUNK", 4);  // 圧縮形式
  write4bytes(aviFile, 4120);                // チャンクサイズ
  //ここから4120バイトです
  {
    write4bytes(aviFile, 4);  // フラグ
    write4bytes(aviFile, 0);  //
    aviFile.write((const uint8_t*)"00dc", 4);
    //ここから0が4108個並ぶ、4バイト単位で1027個
    uint32_t val = 0;
    for (int i = 0; i < 1027; i++) {
      aviFile.write((const uint8_t*)&val, 4);
    }
  }
}

//**************************************************
// OpenDML拡張ヘッダーを書きます
//**************************************************
void OpenDMLHeader(File& aviFile) {
  uint32_t val = 0;
  // JUNKフォーマット
  aviFile.write((const uint8_t*)"JUNK", 4);
  val = 260;
  aviFile.write((const uint8_t*)&val, 4);
  //ここから260バイトです
  {
    aviFile.write((const uint8_t*)"odml", 4);
    aviFile.write((const uint8_t*)"dmlh", 4);
    val = 248;
    aviFile.write((const uint8_t*)&val, 4);
    //ここから0が248個並ぶ、4バイト単位で62個
    val = 0;
    for (int i = 0; i < 62; i++) {
      aviFile.write((const uint8_t*)&val, 4);
    }
  }
}

//**************************************************
// ソフトウエア名称ヘッダーを書きます
//**************************************************
void WriteSoftwareNameHeader(File& aviFile) {
  uint32_t val = 0;
  //LIST ソフトウエア名称
  aviFile.write((const uint8_t*)"LIST", 4);
  write4bytes(aviFile, 26);  // チャンクサイズ
  //ここから26バイトです
  {
    aviFile.write((const uint8_t*)"INFO", 4);
    aviFile.write((const uint8_t*)"ISFT", 4);
    write4bytes(aviFile, 13);
    aviFile.write((const uint8_t*)"CamS3Avi.1.0", 12);  //ソフト名を12バイトで書いた
    aviFile.write((const uint8_t*)&val, 2);
  }

  // JUNKフォーマット
  aviFile.write((const uint8_t*)"JUNK", 4);
  write4bytes(aviFile, 1016);  // チャンクサイズ
  //ここから1016バイトです
  {
    //ここから0が1016個並ぶ、4バイト単位で254個
    for (int i = 0; i < 254; i++) {
      aviFile.write((const uint8_t*)&val, 4);
    }
  }
}