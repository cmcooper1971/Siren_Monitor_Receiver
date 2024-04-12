// 
// 
// 

// Main libraries

#include <TFT_eSPI.h>				// TFT_eSPI library

// Local declarations

#include "mainDisplay.h"
#include "global.h"					// Global
#include "colours.h"				// Colour pallette
#include "screenLayout.h"			// Screen layout

// Debug serial prints

#define DEBUG 0

#if DEBUG==1
#define outputDebug(x); Serial.print(x);
#define outputDebugLn(x); Serial.println(x);
#else
#define outputDebug(x); 
#define outputDebugLn(x); 
#endif


/*-----------------------------------------------------------------*/

// Draw borders

void drawBorder() {

	// Draw layout borders

	tft.drawRect(FRAME1_X, FRAME1_Y, FRAME1_W, FRAME1_H, BLACK);
	tft.drawRect(FRAME2_X, FRAME2_Y, FRAME2_W, FRAME2_H, BLACK);

} // Close function

/*-----------------------------------------------------------------*/

// Draw black boxes when screens change

void drawWhiteBox() {

	// Clear screen by using a black box

	tft.fillRect(FRAME2_X + 1, FRAME2_Y + 30, FRAME2_W - 2, FRAME2_H - 45, WHITE);		// This covers only the graphs and charts, not the system icons to save refresh flicker
	tft.fillRect(FRAME2_X + 1, FRAME2_Y + 1, FRAME2_W - 90, FRAME2_H - 200, WHITE);		// Ths covers the title text per page

} // Close function

/*-----------------------------------------------------------------*/