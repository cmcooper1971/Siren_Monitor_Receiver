// wifiSystem.h

#ifndef _WIFISYSTEM_h
#define _WIFISYSTEM_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

void checkWiFiReset(boolean& wiFiYN);

void wiFiTitle();

bool initWiFi();

void initialiseWiFi();

String getJSONReadings();

void updateWebServer();

#endif

