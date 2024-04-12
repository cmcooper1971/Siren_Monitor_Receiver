// parseData.h

#ifndef _PARSEDATA_h
#define _PARSEDATA_h

// Local definitions

#include "parseData.h"
#include "global.h"
#include "fileOperations.h"
#include "mainDisplay.h"

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

// Extract data

void parseData();

// Write data to CSV & Array

bleSignal parseData(String dataReceived);

// Add entry to array

void addEntryToArray(bleSignal entry);

// Populate array

void populateArrayFromCSV(fs::FS& fs, const char* path, bleSignal* dataEntries, int maxEntries);

// Update table

void updateTable();

// Clear serial buffer

void clearSerialBuffer();

#endif

