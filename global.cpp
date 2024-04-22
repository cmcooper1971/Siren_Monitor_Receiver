// 
// global.cpp
// 

// Main libraries

#include <TFT_eSPI.h>				// TFT_eSPI library
#include <ESP32Time.h>				// NTP time server

// Local declarations

#include "drawBitmap.h"				// Draw bitmaps
#include "global.h"					// Global
#include "colours.h"				// Colour pallette
#include "screenLayout.h"			// Screen layout
#include "icons.h"					// Icons

// Debug serial prints

#define DEBUG 0

#if DEBUG==1
#define outputDebug(x); Serial.print(x);
#define outputDebugLn(x); Serial.println(x);
#else
#define outputDebug(x); 
#define outputDebugLn(x); 
#endif

/*---------------------------------------------------------------- */

// Variables

// Buzzer

int buzzerF;						// Set frequency of the buzzer beep - 1000
int buzzerD;						// Buzzer delay - 75

// Buzzer Pin out

const byte buzzerPin = 34;			// Buzzer enabled / disabled
const byte buzzerP = 21;			// Buzzer

// TFT

TFT_eSPI tft = TFT_eSPI();			// Invoke custom library
boolean screenR = true;				// To limit screen flicker due to unnecessary screen draws.
byte screenMenu = 1;				// Set screen menu

// Time

ESP32Time rtc;						// Configure time settings

/*---------------------------------------------------------------- */

// Functions

// Configure Buzzer

void configureBuzzer() {

	bool buzzerYN = digitalRead(buzzerPin);

	if (buzzerYN == false) {

		buzzerF = 0;	// Set frequency of the buzzer beep.
		buzzerD = 0;	// Set delay of the buzzer beep.
	}

	else
	{
		buzzerF = 1000;	// Set frequency of the buzzer beep.
		buzzerD = 75;	// Set deelay of the buzzer beep.
	}

} // Close function

/*---------------------------------------------------------------- */

// Buzzer

void playTone(byte pin, int freq, byte del) {

    ledcSetup(0, freq, 8);
    ledcAttachPin(pin, 0);
    ledcWriteTone(0, freq);
    delay(del);
    ledcWriteTone(0, 0);

} // Close function

/*---------------------------------------------------------------- */

// Min function

int min(int a, int b) {
    return (a < b) ? a : b;

} // Close function

/*---------------------------------------------------------------- */

// Get and print time

void printLocalTime() {

	// Set time zone

	setenv("TZ","GMT0BST, M3.5.0 / 1, M10.5.0",1);

	tzset();

	// Obtain time

	struct tm timeinfo;

	if (!getLocalTime(&timeinfo)) {

		outputDebugLn("Failed, time set to default.");

		// Set time manually

		rtc.setTime(00, 00, 00, 01, 01, 2021);

		tft.setTextColor(BLACK, WHITE);
		tft.setFreeFont();
		tft.setTextSize(1);
		tft.setCursor(13, 220);
		tft.println("Failed, time set to default.");

		return;
	}

	Serial.println(&timeinfo, "%A, %B %d %Y %H:%M");

	// Text block to over write characters from longer dates when date changes and unit has been running

	tft.setTextColor(BLACK, WHITE);
	tft.setFreeFont();
	tft.setTextSize(1);
	tft.setCursor(150, 220);
	tft.println("                ");

	// Actual date time to display

	tft.setTextColor(BLACK, WHITE);
	tft.setFreeFont();
	tft.setTextSize(1);
	tft.setCursor(13, 220);
	tft.println(&timeinfo, "%A, %B %d %Y %H:%M");


} // Close function

/*-----------------------------------------------------------------*/

// To help print a table for debugging

void printPadded(String str, int width) {
	int len = str.length();
	Serial.print(str);
	for (int i = 0; i < width - len; i++) {
		Serial.print(" ");
	}

} // Close function

/*-----------------------------------------------------------------*/

// Yes / No check

bool areYouSure() {

	tft.fillRect(15, 60, 233, 40, LTRED);
	tft.setFreeFont(&FreeSans12pt7b);
	tft.setTextSize(1);
	tft.setTextColor(DKBLUE); tft.setCursor(60, 88);
	tft.print("Are you sure?");
	
	// Draw buttons

	drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, yesPlease, BUTTON3_W - 2, BUTTON3_H - 2);
	tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);
	drawBitmap(tft, BUTTON4_Y + 1, BUTTON4_X + 1, noThanks, BUTTON4_W - 2, BUTTON4_H - 2);
	tft.drawRect(BUTTON4_X, BUTTON4_Y, BUTTON4_W, BUTTON4_H, WHITE);

	// Check for touch data

	uint16_t x, y;		// variables for touch data.

	while (tft.getTouch(&x, &y)) {

		// Button three

		if ((x > BUTTON3_X) && (x < (BUTTON3_X + BUTTON3_W))) {
			if ((y > BUTTON3_Y) && (y <= (BUTTON3_Y + BUTTON3_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebug("Button 3 hit ");
				outputDebugLn("");

				// Hide buttons

				tft.fillRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);
				tft.fillRect(BUTTON4_X, BUTTON4_Y, BUTTON4_W, BUTTON4_H, WHITE);

				return true;

			}

		}

		// Button four

		if ((x > BUTTON4_X) && (x < (BUTTON4_X + BUTTON4_W))) {
			if ((y > BUTTON4_Y) && (y <= (BUTTON4_Y + BUTTON4_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebug("Button 4 hit ");
				outputDebugLn("");

				// Hide buttons

				tft.fillRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);
				tft.fillRect(BUTTON4_X, BUTTON4_Y, BUTTON4_W, BUTTON4_H, WHITE);

				return false;

			}

		}

	}

} // Close function

/*-----------------------------------------------------------------*/
