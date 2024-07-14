#ifndef _MAKEAVI_H_
#define _MAKEAVI_H_ 1
#include <Arduino.h>

void CreateAviFromJpegs(String& foldername, int width, int height, uint32_t maxJpgSize);
void CreateAviFromJpegs(String& foldername, uint32_t frameCount, uint32_t fps, int width, int height, uint32_t maxJpgSize);
void dword4bytes(uint32_t dword, uint8_t* d4byte);
void write4bytes(File& aviFile, uint32_t dword);
uint32_t GCD(uint32_t a, uint32_t b);
void WriteStreamHeader(File& aviFile, uint32_t width, uint32_t height, long maxJpegSize, uint32_t framePer, uint32_t bunboSec, uint32_t frameCount);
void OpenDMLHeader(File& aviFile);
void WriteSoftwareNameHeader(File& aviFile);

#endif  // _MAKEAVI_H_