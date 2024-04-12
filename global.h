// 
// global.h
// 

#pragma once

#ifndef GLOBAL_H
#define GLOBAL_H

// Main libraries

#include <Arduino.h>
#include <TFT_eSPI.h>				// TFT_eSPI library
#include <ESP32Time.h>				// NTP time server

// Local declations

#include "colours.h"				// Colour pallette
#include "screenLayout.h"			// Screen layout

/*---------------------------------------------------------------- */

// Variables

// Buzzer

extern int buzzerF;
extern int buzzerD;
extern boolean buzzerYN;

// Buzzer pin outs

extern const byte buzzerPin;
extern const byte buzzerP;

// TFT

extern TFT_eSPI tft;
extern boolean screenR;
extern byte screenMenu;

// Time

extern ESP32Time rtc;

/*---------------------------------------------------------------- */

// Functions

// Configure buzzer

void configureBuzzer();

// Buzzer

void playTone(byte pin, int freq, byte del);

// Min

int min(int a, int b);

// Time

void printLocalTime();

// Print padded table

void printPadded(String str, int width);

// Yes / No control

bool areYouSure();

/*---------------------------------------------------------------- */

#endif
