// touchCalibrate.h

#ifndef _TOUCHCALIBRATE_h
#define _TOUCHCALIBRATE_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

void touch_calibrate(TFT_eSPI& tft);

#endif

