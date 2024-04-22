/*
 Name:		Siren_Monitor_Receiver.ino
 Created:	3/19/2024 8:19:49 PM
 Author:	Christopher Cooper
*/

// Include libraries

#include <WiFi.h>					// WiFi library
#include <ESP32Time.h>				// NTP time server
#include <AsyncTCP.h>				// TCP socket
#include <ESPAsyncWebSrv.h>			// Web server
#include <ArduinoJson.h>			// ArduinoJson
#include <ArduinoJson.hpp>			// ArduinoJson
#include <SPI.h>					// SPI library
#include <FS.h>						// Files system library
#include <SD.h>						// SD Card library
#include <SPIFFS.h>					// Spiffs library
#include <TFT_eSPI.h>				// Bodmer TFT library
#include <EEPROM.h>					// EEPROM library

// Local declarations

#include "touchCalibrate.h"			// Calibrate touch screen.
#include "drawBitmap.h"				// Draw bitmaps
#include "colours.h"				// Colour pallette
#include "startScreen.h"			// Start image
#include "screenLayout.h"			// Screen layout
#include "icons.h"					// More icons
#include "Free_Fonts.h"				// Additional fonts
#include "global.h"					// Global variables
#include "wifiSystem.h"				// WiFi & Web Server
#include "sensorFunctions.h"		// Sensor checking (is Arduino Nano responding?)
#include "fileOperations.h"			// File operations
#include "mainDisplay.h"			// Display layout
#include "parseDataReceived.h"		// CSV file operations

// Debug serial prints

#define DEBUG 0

#if DEBUG==1
#define outputDebug(x); Serial.print(x);
#define outputDebugLn(x); Serial.println(x);
#else
#define outputDebug(x); 
#define outputDebugLn(x); 
#endif

// TFT SPI interface for ESP32 using TFT-eSPI

#define TFT_CS   5
#define TFT_DC   4
#define TFT_LED  15
#define TFT_MOSI 23
#define TFT_CLK  18
#define TFT_RST  22
#define TFT_MISO 19
#define TOUCH_CS 14

// SD card interface

#define SCK  18
#define MISO  19
#define MOSI  23
#define sdCS  25

// Pin outs

const byte espInterrupt = 26;		// Digital pin connected to interrupt signal pin on Arduino Nano
const byte nanoReset = 33;			// Reset Arduino BLE Sense
const byte wiFiResetPin = 39;		// Reset WiFi
const byte calTouchScreenPin = 35;	// Calibrate touch screen
// const byte buzzerPin = 34;		// Buzzer enabled / disabled -  See global.h & global.cpp
// const byte buzzerP = 21;			// Buzzer - See global.h & global.cpp

// Invoke TFT library

// See global.h & global.cpp

// TFT back light sleep

unsigned long sleepT = 0;
unsigned long sleepTime = 300000;	// Reset to 300,000 when finished with design (5 minutes sleep time).

// TFT calibration

uint16_t calData[5];				// Touch screen calibration data.
boolean calTouchScreen = false;		// Change flag to trigger calibration function

// Serial 2 Interface

#define RXD2 16
#define TXD2 17

// Configure time settings

const char* ntpServer = "2.uk.pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

// Interrupt from Arduino Nano BLE

const unsigned long interruptCheckPeriod = 10000;		// Interrupt checking period
volatile boolean interruptDetected = false;				// Flag to indicate interrupt detection
const byte interruptCount = 5;							// Failed interrupt setting

// Time and date update & update web server

const unsigned long timeCheckPeriod = 50000;			// Interval to wait between time and date update
static unsigned long timeCheck = 0;						// Updated time

//*---------------------------------------------------------------- */

// Interrupt from Arduino Nano BLE Sense

void IRAM_ATTR handleInterrupt() {

	interruptDetected = true;

} // Close function

/*-----------------------------------------------------------------*/

void setup() {

	// Setup Serial

	Serial.begin(115200);

	delay(100);

	outputDebugLn("");
	outputDebugLn("Serial started successfully...");

	// Setup Serial 1 for Nano BLE Communications

	Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);	// initialize UART

	delay(100);

	outputDebugLn("");
	outputDebugLn("Serial 2 started successfully...");

	// Set pin modes

	pinMode(nanoReset, OUTPUT);			// Output for resetting Nano BLE Sense
	pinMode(TFT_LED, OUTPUT);			// Output for LCD back light
	pinMode(espInterrupt, INPUT);		// Input to detect if Arduino Nano BLE Sense interrupt signal
	pinMode(wiFiResetPin, INPUT);		// Input to detect if WiFi to be reset
	pinMode(calTouchScreenPin, INPUT);	// Input to detect if screen to be calibrated
	pinMode(buzzerPin, INPUT);			// Input to detect if buzzer is enabled

	// Set Arduino Nano BLE Sense reset pin

	digitalWrite(nanoReset, HIGH);

	// Attach interrupt

	attachInterrupt(digitalPinToInterrupt(espInterrupt), handleInterrupt, RISING);

	// Switch off TFT LED back light

	digitalWrite(TFT_LED, HIGH);			// Output for LCD back light

	// Set all SPI chip selects to HIGH to stablise SPI bus

	digitalWrite(TOUCH_CS, HIGH);			// Touch controller chip select
	digitalWrite(TFT_CS, HIGH);				// TFT screen chip select

	// Initialize SPIFFS

	if (!SPIFFS.begin(true)) {

		outputDebugLn("");
		outputDebugLn("An error has occurred while mounting SPIFFS");
	}

	else

	{
		outputDebugLn("");
		outputDebugLn("SPIFFS mounted successfully");
	}

	// Initialize SD card

	digitalWrite(sdCS, LOW);				// TFT screen chip select
	digitalWrite(TFT_CS, HIGH);				// TFT screen chip select

	if (!SD.begin(sdCS)) {
		outputDebugLn("");
		outputDebugLn("SD initialization failed!");

	}

	else {
		outputDebugLn("");
		outputDebugLn("SD initialized...");

	}

	delay(100);

	// Initialize EEPROM

	if (!EEPROM.begin(512)) {

		outputDebugLn("");
		outputDebugLn("EEPROM initialization failed!");

	}

	else {
		outputDebugLn("");
		outputDebugLn("EEPROM initialized...");

	}

	// Initialize buzzer

	configureBuzzer();

	playTone(buzzerP, buzzerF, buzzerD);

	// Initialise TFT display

	tft.begin();

	// Setup TFT

	tft.setRotation(3);
	tft.setCursor(0, 0);
	
	digitalWrite(TFT_LED, LOW);				// LOW to turn backlight on

	// Start up screen image and title

	tft.fillScreen(WHITE);

	drawBitmap(tft, 40, 96, startScreen, 128, 128);

	tft.setFreeFont(&FreeSans12pt7b);
	tft.setTextSize(1);
	tft.setTextColor(BLACK);
	tft.setCursor(90, 190);
	tft.println("Siren Monitor");
	tft.setFreeFont();
	tft.setTextSize(1);
	tft.setCursor(150, 200);
	tft.println("Mk-1");

	delay(2000);

	// Clear screen

	tft.fillScreen(WHITE);

	// Reset Nano BLE Sense after power up

	digitalWrite(nanoReset, LOW);
	delay(100);
	digitalWrite(nanoReset, HIGH);

	// Calibrate touch screen

	//bool calTouchScreen = digitalRead(calTouchScreenPin);

	//outputDebugLn("");
	//outputDebug("Calibrate Touch: ");
	//outputDebug(calTouchScreen);
	//outputDebugLn("");

	//if (calTouchScreen == false) {

	//	touch_calibrate(tft);
	//}

	// Load calibration data from EEPROM

	int eeCalYNAddress = 304;					// EEPROM address for touch screen calibration enabled disabled.
	int eeCalDataAddress0 = 308;				// EEPROM address for touch screen calibration data.
	int eeCalDataAddress1 = 312;				// EEPROM address for touch screen calibration data.
	int eeCalDataAddress2 = 316;				// EEPROM address for touch screen calibration data.
	int eeCalDataAddress3 = 320;				// EEPROM address for touch screen calibration data.
	int eeCalDataAddress4 = 324;				// EEPROM address for touch screen calibration data.

	EEPROM.get(eeCalYNAddress, calTouchScreen);				// Load touch screen calibration data.
	EEPROM.get(eeCalDataAddress0, calData[0]);
	EEPROM.get(eeCalDataAddress1, calData[1]);
	EEPROM.get(eeCalDataAddress2, calData[2]);
	EEPROM.get(eeCalDataAddress3, calData[3]);
	EEPROM.get(eeCalDataAddress4, calData[4]);

	EEPROM.commit();

	// Output calibration data to serial for checking.

	outputDebug("Calibration Data: ");

	for (uint8_t i = 0; i < 5; i++) {

		outputDebug(calData[i]);
		if (i < 4) outputDebug(", ");
	}

	// Set calibration

	tft.setTouch(calData);

	outputDebugLn("");

	// Check WiFi Reset

	bool wiFiReset = digitalRead(wiFiResetPin);

	outputDebugLn("");
	outputDebug("WiFi Reset: ");
	outputDebug(wiFiReset);
	outputDebugLn();

	checkWiFiReset(wiFiReset);

	// Intialise WiFIi

	initialiseWiFi();

	// Initialize time and get the time

	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	printLocalTime();

	outputDebugLn("");

	// SD Card Diagnostics

	uint8_t cardType = SD.cardType();

	if (cardType == CARD_NONE) {
		outputDebugLn("");
		outputDebugLn("No SD card attached");

	}

	else {
		outputDebugLn("");
		outputDebug("SD Card type: ");
		outputDebugLn(cardType);

	}

	delay(100);

	outputDebugLn("");
	listDir(SD, "/", 0);

	outputDebugLn("");
	uint64_t cardSize = SD.cardSize() / (1024 * 1024);
	Serial.printf("SD Card Size: %lluMB\n", cardSize);

	delay(100);

	outputDebugLn("");
	outputDebugLn("initialisation done.");

	delay(100);

	outputDebugLn("");

	// Check if CSV file exists

	if (!SD.exists(fileName)) {

		// File doesn't exist, create it

		if (createCSVFile(fileName)) {

			outputDebugLn("");
			outputDebugLn("CSV file created successfully.");
			outputDebugLn("");

		}

		else {

			outputDebugLn("");
			outputDebugLn("Error creating CSV file!");
			outputDebugLn("");

		}
	}

	else {

		outputDebugLn("");
		outputDebugLn("CSV file already exists.");
		outputDebugLn("");

	}

	delay(100);

	// Populate temporary screen array from the CSV file

	populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);

	delay(100);

	// Draw border and buttons at start.

	tft.fillScreen(WHITE);								// Clear screen

	drawBorder();										// Screen border layouts.

	Serial.println("");

	printLocalTime();									// Redisplay date & time

	// Status icons - SD Card

	if (!SD.exists(fileName)) (

		drawBitmap(tft, SDCARD_ICON_Y, SDCARD_ICON_X, sdCardRed, SDCARD_ICON_W, SDCARD_ICON_H));


	else drawBitmap(tft, SDCARD_ICON_Y, SDCARD_ICON_X, sdCardGreen, SDCARD_ICON_W, SDCARD_ICON_H);

	// Status icons - WiFi

	if ((WiFi.status() != WL_CONNECTED)) {				// Update WiFi icon

		drawBitmap(tft, WIFI_ICON_Y, WIFI_ICON_X, wiFiAmber, WIFI_ICON_W, WIFI_ICON_H); 

	}

	else drawBitmap(tft, WIFI_ICON_Y, WIFI_ICON_X, wiFiGreen, WIFI_ICON_W, WIFI_ICON_H);

	// Status icons - Sensor

	drawBitmap(tft, PULSE_ICON_Y, PULSE_ICON_X, pulseGreen, PULSE_ICON_W, PULSE_ICON_H);

	// Main title

	tft.setFreeFont(&FreeSans9pt7b);
	tft.setTextSize(1);
	tft.setTextColor(BLACK);
	tft.setCursor(13, 26);
	tft.print("Siren Monitor");

	// Clear serial buffer

	clearSerialBuffer();

	// Update TFT table

	updateTable();

	// Update webserver

	updateWebServer();

} // Close setup

/*---------------------------------------------------------------- */

void loop() {

	// Draw screen layout

	if (screenMenu == true) {

		drawBitmap(tft, BUTTON1_Y + 1, BUTTON1_X + 1, categoriseEvents, BUTTON1_W - 2, BUTTON1_H - 2);
		tft.drawRect(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, WHITE);

		drawBitmap(tft, BUTTON2_Y + 1, BUTTON2_X + 1, manualEntry, BUTTON2_W - 2, BUTTON2_H - 2);
		tft.drawRect(BUTTON2_X, BUTTON2_Y, BUTTON2_W, BUTTON2_H, WHITE);

		drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, deleteLastLine, BUTTON3_W - 2, BUTTON3_H - 2);
		tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);

		screenMenu = false;

	}

	// TFT backlight sleep mode

	if (millis() >= sleepT + sleepTime) {

		digitalWrite(TFT_LED, HIGH);		// Output for LCD back light.

	}

	// Check for touch data

	uint16_t x, y;		// variables for touch data.

	if (tft.getTouch(&x, &y)) {

		// Restart TFT backlight sleep timer.

		sleepT = millis();

		// Button one

		if ((x > BUTTON1_X) && (x < (BUTTON1_X + BUTTON1_W))) {
			if ((y > BUTTON1_Y) && (y <= (BUTTON1_Y + BUTTON1_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebug("Button 1 pressed ");
				outputDebugLn("");

				tft.fillRect(15, 60, 233, 40, LTRED);
				tft.setFreeFont(&FreeSans12pt7b);
				tft.setTextSize(1);
				tft.setTextColor(DKBLUE);
				tft.setCursor(25, 88);
				tft.print("Update categories?");
				
				delay(1500);

				// Check

				bool response = areYouSure();

				// Update screen layout

				if (response == true) {

					drawBitmap(tft, BUTTON1_Y + 1, BUTTON1_X + 1, policeCar, BUTTON1_W - 2, BUTTON1_H - 2);
					tft.drawRect(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, WHITE);
					drawBitmap(tft, BUTTON2_Y + 1, BUTTON2_X + 1, ambulance, BUTTON2_W - 2, BUTTON2_H - 2);
					tft.drawRect(BUTTON2_X, BUTTON2_Y, BUTTON2_W, BUTTON2_H, WHITE);
					drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, fireEngine, BUTTON3_W - 2, BUTTON3_H - 2);
					tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);
					drawBitmap(tft, BUTTON4_Y + 1, BUTTON4_X + 1, falsePositive, BUTTON4_W - 2, BUTTON4_H - 2);
					tft.drawRect(BUTTON4_X, BUTTON4_Y, BUTTON4_W, BUTTON4_H, WHITE);
					
					categorizeEntries(SD, fileName);

					drawBitmap(tft, BUTTON1_Y + 1, BUTTON1_X + 1, categoriseEvents, BUTTON1_W - 2, BUTTON1_H - 2);
					tft.drawRect(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, WHITE);
					
					drawBitmap(tft, BUTTON2_Y + 1, BUTTON2_X + 1, manualEntry, BUTTON2_W - 2, BUTTON2_H - 2);
					tft.drawRect(BUTTON2_X, BUTTON2_Y, BUTTON2_W, BUTTON2_H, WHITE);

					drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, deleteLastLine, BUTTON3_W - 2, BUTTON3_H - 2);
					tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);

					tft.fillRect(BUTTON4_X, BUTTON4_Y, BUTTON4_W, BUTTON4_H, WHITE);

					populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);
					updateTable();

				}

				else {

					drawBitmap(tft, BUTTON1_Y + 1, BUTTON1_X + 1, categoriseEvents, BUTTON1_W - 2, BUTTON1_H - 2);
					tft.drawRect(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, WHITE);
					
					populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);
					updateTable();
				}
	
			}

		}

		// Button Two

		if ((x > BUTTON2_X) && (x < (BUTTON2_X + BUTTON2_W))) {
			if ((y > BUTTON2_Y) && (y <= (BUTTON2_Y + BUTTON2_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebug("Button 2 pressed ");
				outputDebugLn("");

				tft.fillRect(15, 60, 233, 40, LTRED);
				tft.setFreeFont(&FreeSans12pt7b);
				tft.setTextSize(1);
				tft.setTextColor(DKBLUE);
				tft.setCursor(33, 88);
				tft.print("Add manual entry?");
				
				delay(1500);

				// Check

				bool response = areYouSure();

				// Update screen layout

				if (response == true) {

					tft.fillRect(15, 60, 233, 40, LTRED);
					tft.setFreeFont(&FreeSans12pt7b);
					tft.setTextSize(1);
					tft.setTextColor(DKBLUE);
					tft.setCursor(50, 88);
					tft.print("Select category");

					drawBitmap(tft, BUTTON1_Y + 1, BUTTON1_X + 1, policeCar, BUTTON1_W - 2, BUTTON1_H - 2);
					tft.drawRect(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, WHITE);
					drawBitmap(tft, BUTTON2_Y + 1, BUTTON2_X + 1, ambulance, BUTTON2_W - 2, BUTTON2_H - 2);
					tft.drawRect(BUTTON2_X, BUTTON2_Y, BUTTON2_W, BUTTON2_H, WHITE);
					drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, fireEngine, BUTTON3_W - 2, BUTTON3_H - 2);
					tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);
					drawBitmap(tft, BUTTON4_Y + 1, BUTTON4_X + 1, falsePositive, BUTTON4_W - 2, BUTTON4_H - 2);
					tft.drawRect(BUTTON4_X, BUTTON4_Y, BUTTON4_W, BUTTON4_H, WHITE);

					addManualEntry(SD, fileName);

					drawBitmap(tft, BUTTON1_Y + 1, BUTTON1_X + 1, categoriseEvents, BUTTON1_W - 2, BUTTON1_H - 2);
					tft.drawRect(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, WHITE);

					drawBitmap(tft, BUTTON2_Y + 1, BUTTON2_X + 1, manualEntry, BUTTON2_W - 2, BUTTON2_H - 2);
					tft.drawRect(BUTTON2_X, BUTTON2_Y, BUTTON2_W, BUTTON2_H, WHITE);

					drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, deleteLastLine, BUTTON3_W - 2, BUTTON3_H - 2);
					tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);

					tft.fillRect(BUTTON4_X, BUTTON4_Y, BUTTON4_W, BUTTON4_H, WHITE);

					populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);
					updateTable();

					tft.fillRect(15, 60, 233, 40, LTRED);
					tft.setFreeFont(&FreeSans12pt7b);
					tft.setTextSize(1);
					tft.setTextColor(DKBLUE);
					tft.setCursor(25, 88);
					tft.print("Manual entry added");

					delay(1500);

					populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);
					updateTable();

				}

			}

			else {

				drawBitmap(tft, BUTTON1_Y + 1, BUTTON1_X + 1, categoriseEvents, BUTTON1_W - 2, BUTTON1_H - 2);
				tft.drawRect(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, WHITE);

				drawBitmap(tft, BUTTON2_Y + 1, BUTTON2_X + 1, manualEntry, BUTTON2_W - 2, BUTTON2_H - 2);
				tft.drawRect(BUTTON2_X, BUTTON2_Y, BUTTON2_W, BUTTON2_H, WHITE);

				drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, deleteLastLine, BUTTON3_W - 2, BUTTON3_H - 2);
				tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);

				populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);
				updateTable();
			}

		}

		// Button three

		if ((x > BUTTON3_X) && (x < (BUTTON3_X + BUTTON3_W))) {
			if ((y > BUTTON3_Y) && (y <= (BUTTON3_Y + BUTTON3_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebug("Button 3 pressed ");
				outputDebugLn("");

				tft.fillRect(15, 60, 233, 40, LTRED);
				tft.setFreeFont(&FreeSans12pt7b);
				tft.setTextSize(1);
				tft.setTextColor(DKBLUE);
				tft.setCursor(40, 88);
				tft.print("Delete last entry?");
				
				delay(1500);

				// Check

				bool response = areYouSure();

				drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, deleteLastLine, BUTTON3_W - 2, BUTTON3_H - 2);
				tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);

				// Update screen layout

				if (response == true) {

					deleteLastEntry(SD, fileName);

					tft.fillRect(15, 60, 233, 40, LTRED);
					tft.setFreeFont(&FreeSans12pt7b);
					tft.setTextSize(1);
					tft.setTextColor(DKBLUE);
					tft.setCursor(35, 88);
					tft.print("Last entry deleted");

					delay(1500);

					drawBitmap(tft, BUTTON1_Y + 1, BUTTON1_X + 1, categoriseEvents, BUTTON1_W - 2, BUTTON1_H - 2);
					tft.drawRect(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, WHITE);

					drawBitmap(tft, BUTTON2_Y + 1, BUTTON2_X + 1, manualEntry, BUTTON2_W - 2, BUTTON2_H - 2);
					tft.drawRect(BUTTON2_X, BUTTON2_Y, BUTTON2_W, BUTTON2_H, WHITE);

					drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, deleteLastLine, BUTTON3_W - 2, BUTTON3_H - 2);
					tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);

					tft.fillRect(BUTTON4_X, BUTTON4_Y, BUTTON4_W, BUTTON4_H, WHITE);

					for (int i = 0; i < maxEntries; i++) {
						dataEntries[i].title = "";
						dataEntries[i].date = "";
						dataEntries[i].time = "";
						dataEntries[i].category = "";
						dataEntries[i].percentage = "";
					}

					populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);
					updateTable();

				}

				else {

					drawBitmap(tft, BUTTON1_Y + 1, BUTTON1_X + 1, categoriseEvents, BUTTON1_W - 2, BUTTON1_H - 2);
					tft.drawRect(BUTTON1_X, BUTTON1_Y, BUTTON1_W, BUTTON1_H, WHITE);

					drawBitmap(tft, BUTTON2_Y + 1, BUTTON2_X + 1, manualEntry, BUTTON2_W - 2, BUTTON2_H - 2);
					tft.drawRect(BUTTON2_X, BUTTON2_Y, BUTTON2_W, BUTTON2_H, WHITE);

					drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, deleteLastLine, BUTTON3_W - 2, BUTTON3_H - 2);
					tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);

					tft.fillRect(BUTTON4_X, BUTTON4_Y, BUTTON4_W, BUTTON4_H, WHITE);

					for (int i = 0; i < maxEntries; i++) {
						dataEntries[i].title = "";
						dataEntries[i].date = "";
						dataEntries[i].time = "";
						dataEntries[i].category = "";
						dataEntries[i].percentage = "";
					}

					populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);
					updateTable();
				}

			}

		}
                 
		// Main area - Wake up TFT backlight

		if ((x > FRAME2_X) && (x < (FRAME2_X + FRAME2_W))) {
			if ((y > FRAME2_Y) && (y <= (FRAME2_Y + FRAME2_H))) {

				playTone(buzzerP, buzzerF, buzzerD);
				digitalWrite(TFT_LED, LOW);				// Turn TFT backlight on from sleep

				outputDebug("Main frame pressed");
				outputDebugLn("");

			}

		}

		// SD Card

		if ((x > SDCARD_ICON_X) && (x < (SDCARD_ICON_X + SDCARD_ICON_W))) {
			if ((y > SDCARD_ICON_Y) && (y <= (SDCARD_ICON_Y + SDCARD_ICON_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebug("SD Icon Pressed");
				outputDebugLn("");

				tft.fillRect(15, 60, 233, 40, LTRED);
				tft.setFreeFont(&FreeSans12pt7b);
				tft.setTextSize(1);
				tft.setTextColor(DKBLUE);
				tft.setCursor(60, 88);
				tft.print("Data file copy");

				delay(1500);

				bool response = (createDataCopy(SD, fileName));

				if  (response == true) { 	// Take a copy of the data file, incrementing number

					drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, deleteLastLine, BUTTON3_W - 2, BUTTON3_H - 2);
					tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);
						
					tft.fillRect(15, 60, 233, 40, LTRED);
					tft.setFreeFont(&FreeSans12pt7b);
					tft.setTextSize(1);
					tft.setTextColor(DKBLUE);
					tft.setCursor(50, 88);
					tft.print("Data file copied");

					delay(1500);

				}

				else {

					drawBitmap(tft, BUTTON3_Y + 1, BUTTON3_X + 1, deleteLastLine, BUTTON3_W - 2, BUTTON3_H - 2);
					tft.drawRect(BUTTON3_X, BUTTON3_Y, BUTTON3_W, BUTTON3_H, WHITE);

					tft.fillRect(15, 60, 233, 40, LTRED);
					tft.setFreeFont(&FreeSans12pt7b);
					tft.setTextSize(1);
					tft.setTextColor(DKBLUE);
					tft.setCursor(40, 88);
					tft.print("Abort copy / error");

					delay(1500);

				}

				populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);
				updateTable();
			}

		}

	}

	// Check for serial communication from Nano BLE

	if (Serial2.available()) {

		parseData();

	}

	// If event detected from Arduino Nano is received, update TFT table

	if (newDataReceived == true) {

		populateArrayFromCSV(SD, fileName, dataEntries, maxEntries);
		updateTable();

	}

	// Update time, date and refresh webserver

	if (millis() - timeCheck >= timeCheckPeriod) {

		printLocalTime();									// Redisplay date & time

		timeCheck = millis();

		// Update web server

		updateWebServer();

	}

	// Check Arduino Nano BLE Sense is alive

	sensorCheckCall(interruptDetected, interruptCheckPeriod, interruptCount, nanoReset);

} // Close loop

/*---------------------------------------------------------------- */
