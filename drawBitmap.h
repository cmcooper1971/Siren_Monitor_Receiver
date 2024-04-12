// drawBitmap.h

#ifndef _DRAWBITMAP_h
#define _DRAWBITMAP_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

void drawBitmap(TFT_eSPI& tft, int x, int y, const uint16_t* bitmap, int bw, int bh);

#endif

