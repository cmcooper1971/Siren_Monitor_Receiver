// 
// fileOperations.h
// 

#ifndef _FILEOPERATIONS_h
#define _FILEOPERATIONS_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

/*---------------------------------------------------------------- */

// File name

extern const char* fileName;

// Data entry array

const int maxEntries = 10; // Define maxEntries globally

// Data array

struct bleSignal {							// Storing received data from Nano BLE
	String title;
	String date;
	String time;
	String category;
	String percentage;
};

extern String dataReceived;						// Storing received data from Nano BLE
extern bleSignal dataEntries[maxEntries];		// Array to store the last 10 entries
extern boolean newDataReceived;					// Flag for each time serial data is received

/*---------------------------------------------------------------- */

// Main libraries

#include <FS.h>						// Files system library
#include <SD.h>						// SD Card library
#include <SPIFFS.h>					// Spiffs library

// Local declarations

#include "fileOperations.h"

// Functions

// Read file

String readFile(fs::FS& fs, const char* path);

// Write file

void writeFile(fs::FS& fs, const char* path, const char* message);

// List directory

void listDir(fs::FS& fs, const char* dirname, uint8_t levels);

// Create CSV

bool createCSVFile(const char* fileName);

// Append file

void appendFile(fs::FS& fs, const char* path, bleSignal newData);

// Convert String to CSV line

String toCSVLine(const bleSignal& data);

// Wait for category selection

String waitForCategorySelection();

// Categorise entries

void categorizeEntries(fs::FS& fs, const char* path);

// Add manual entry

void addManualEntry(fs::FS& fs, const char* path);

// Create CSV file copy

bool createDataCopy(fs::FS& fs, const char* path);

// Delete last entry

void deleteLastEntry(fs::FS& fs, const char* path);

#endif

