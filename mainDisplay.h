// mainDisplay.h

#ifndef _MAINDISPLAY_h
#define _MAINDISPLAY_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"

// Main libraries

#include <TFT_eSPI.h>				// TFT_eSPI library

// Local declations

#include "global.h"
#include "colours.h"				// Colour pallette
#include "screenLayout.h"			// Screen layout

#else
	#include "WProgram.h"
#endif

void drawBorder();

void drawWhiteBox();

#endif

