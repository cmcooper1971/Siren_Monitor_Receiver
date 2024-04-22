// 
// 
// 

// Location declarations

#include "parseDataReceived.h"
#include "global.h"
#include "fileOperations.h"
#include "mainDisplay.h"

// Debug serial prints

#define DEBUG 0

#if DEBUG==1
#define outputDebug(x); Serial.print(x);
#define outputDebugLn(x); Serial.println(x);
#else
#define outputDebug(x); 
#define outputDebugLn(x); 
#endif

// Variables

int numEntries = 0;												// Number of entries currently in the array
unsigned long lastEventTime = 0;								// Time since the last event was recognized
const unsigned long firstDetectionTimeCheckPeriod = 5000;		// Period to wait before recognizing a new event

unsigned long waitSinceLastEventTime = 0;					// Time since the next event wait period
const unsigned long waitDetectionPeriod = 10000;			// Period to wait before recognizing a new event

/*-----------------------------------------------------------------*/

// Parse data from serial

void parseData() {

	// Event count

	static byte eventCount = 0;
	static const byte totalEvents = 2;

	// Set current millis time

	unsigned long currentMillis = millis();

	// Read data into String

	dataReceived = "";
	dataReceived = Serial2.readStringUntil('%');		// '%' isnt included in the data

	outputDebug("Data received from Serial 2: ");
	outputDebugLn(dataReceived);
	outputDebugLn("");

	// Parse the received data

	bleSignal newData = parseDataS(dataReceived);

	// Check if the title and percentage are not blank

	if (!newData.title.isEmpty() || !newData.percentage.isEmpty()) {

		// Check if signal is within first wait period

		if (currentMillis - waitSinceLastEventTime >= waitDetectionPeriod) {

			// Check if over all period has exceeded and reset event count

			if ((currentMillis - lastEventTime >= firstDetectionTimeCheckPeriod)) {

				eventCount = 0;

				Serial.print("Reset Event Count:  ");
				Serial.println(eventCount);

			}

			// Increment event count

			eventCount++;

			// Update the last event time

			lastEventTime = currentMillis;

			Serial.print("Event Count:          ");
			Serial.println(eventCount);

			// Check if event count has reached preset quantity and within time period

			if (eventCount >= totalEvents && (currentMillis - lastEventTime <= firstDetectionTimeCheckPeriod)) {

				Serial.print("Event Count:          ");
				Serial.println(eventCount);

				Serial.println("Event detected");

				// Update display

				newDataReceived = true;

				// Add the new entry to the array

				addEntryToArray(newData);

				// Update CSV file with the new entry

				appendFile(SD, fileName, newData);

				// Update the last event time

				lastEventTime = currentMillis;

				// clear event count

				Serial.print("Reset Event Count*: ");
				Serial.println(eventCount);

				waitSinceLastEventTime = currentMillis;

				eventCount = 0;

			}

			// Display received data

			if (DEBUG == 1) {

				Serial.println("");
				Serial.println("New Data in Parse Data Received");
				Serial.println("");
				Serial.print("Title:      ");
				Serial.println(newData.title);
				Serial.print("Date:       ");
				Serial.println(newData.date);
				Serial.print("Time:       ");
				Serial.println(newData.time);
				Serial.print("Catagory:   ");
				Serial.println(newData.category);
				Serial.print("Accuracy:   ");
				Serial.println(newData.percentage);
				Serial.println();

			}

		}

		else {

			// After successful event, wait 10 seconds before a new event can be recorded

			Serial.print("Wait 10 seconds...");
			Serial.println("");

		}

	}

	else {
		outputDebugLn("Error: Title or percentage is blank!");
		clearSerialBuffer();

		return;

	}

}  // Close function

/*-----------------------------------------------------------------*/

// CSV data parsing

bleSignal parseDataS(String dataReceived) {

	bleSignal newData;

	// Ensure struct is empty

	newData.title = "";
	newData.date = "";
	newData.time = "";
	newData.category = "";
	newData.percentage = "";
	
	// Get current local time

	struct tm timeinfo;
	if (getLocalTime(&timeinfo)) {

		// Extract date from timeinfo

		char date[11]; // Format: DD-MM-YYYY

		snprintf(date, sizeof(date), "%02d-%02d-%04d", timeinfo.tm_mday, timeinfo.tm_mon + 1, timeinfo.tm_year + 1900);

		// Assign date to newData

		newData.date = String(date);

		// Extract time from timeinfo

		char time[9]; // Format: HH:MM:SS

		snprintf(time, sizeof(time), "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

		// Assign time to newData

		newData.time = String(time);
	}

	// Find the positions of the commas

	int commaIndex1 = dataReceived.indexOf(',');

	int commaIndex2 = dataReceived.indexOf(',', commaIndex1 + 1);

	if (commaIndex1 != -1 && commaIndex2 != -1) {

		// Extract and trim title - Trim is needed to remove control characters from serial inputs

		newData.title = dataReceived.substring(0, commaIndex1);

		newData.title.trim();		// Properly apply trim to the extracted substring

		// Extract and trim category

		newData.category = dataReceived.substring(commaIndex1 + 1, commaIndex2);

		newData.category.trim();	// Properly apply trim to the extracted substring

		// Extract and trim percentage

		newData.percentage = dataReceived.substring(commaIndex2 + 1);

		newData.percentage.trim();  // Properly apply trim to the extracted substring
		newData.percentage += '%';	// Append percentage sign

	}

	return newData;

}  // Close function

/*-----------------------------------------------------------------*/

// Update temporary array

void addEntryToArray(bleSignal entry) {

	// Shift existing entries to make space for the new entry

	for (int i = maxEntries - 1; i > 0; i--) {
		dataEntries[i] = dataEntries[i - 1];
	}

	// Add the new entry to the first position in the array

	dataEntries[0] = entry;

	// Update the number of entries

	numEntries = min(numEntries + 1, maxEntries);

} // Close function

/*-----------------------------------------------------------------*/

// Populate temporary array from CSV file

void populateArrayFromCSV(fs::FS& fs, const char* path, bleSignal* dataEntries, int maxEntries) {

	// Open the CSV file

	File file = fs.open(path, FILE_READ);

	if (!file) {
		outputDebugLn("");
		outputDebugLn("Failed to open CSV file");
		return;
	}

	// Calculate the total number of rows (excluding the header)

	int totalRows = 0;

	while (file.available()) {
		if (file.readStringUntil('\n').length() > 0) {
			totalRows++;
		}
	}

		outputDebugLn("");
		outputDebug("Total number of rows in CSV file: ");
		outputDebugLn(totalRows);
		outputDebugLn("");

	// Start reading from the beginning of the file

	file.seek(0);

	// Move to the desired starting row

	int startRow = totalRows - maxEntries;

	for (int i = 0; i < (startRow); i++) {
		String line = file.readStringUntil('\n');

		outputDebug("Parse array from CSV: ")
		outputDebug("Starting line: ");
		outputDebugLn(line);
		outputDebugLn("");

	}

	int position = 9;

	if (totalRows < 10) {

		position = totalRows - 1;
	}

	else position = 9;

	// Read the rows and populate the array

	// (int i = 0; i < maxEntries; i++)

	for (int i = position; i > -1; i--) {

		// Read the next line from the file

		String line = file.readStringUntil('\r\n');

		// Parse the line and populate the array

		int commaIndex1 = line.indexOf(',');
		int commaIndex2 = line.indexOf(',', commaIndex1 + 1);
		int commaIndex3 = line.indexOf(',', commaIndex2 + 1);
		int commaIndex4 = line.indexOf(',', commaIndex3 + 1);

		if (commaIndex1 != -1 && commaIndex2 != -1 && commaIndex3 != -1 && commaIndex4 != -1) {

			// Extract data from the line

			dataEntries[i].title = line.substring(0, commaIndex1);
			dataEntries[i].date = line.substring(commaIndex1 + 1, commaIndex2);
			dataEntries[i].time = line.substring(commaIndex2 + 1, commaIndex3);
			dataEntries[i].category = line.substring(commaIndex3 + 1, commaIndex4);
			dataEntries[i].percentage = line.substring(commaIndex4 + 1);

		}

		outputDebug("Title: ");
		outputDebug(dataEntries[i].title);
		outputDebug(", Date: ");
		outputDebug(dataEntries[i].date);
		outputDebug(", Time: ");
		outputDebug(dataEntries[i].time);
		outputDebug(", Catagory: ");
		outputDebug(dataEntries[i].category);
		outputDebug(", Accuracy: ");
		outputDebugLn(dataEntries[i].percentage);

		numEntries++;

	}

	outputDebugLn("");

	// Close the file

	file.close();

} // Close function

/*-----------------------------------------------------------------*/

// Update table on TFT display

void updateTable() {

	drawWhiteBox();

	tft.setFreeFont(&FreeSans9pt7b);
	tft.setTextSize(1);
	tft.setTextColor(BLACK);
	tft.setCursor(13, 26);
	tft.print("Siren Monitor");

	tft.setFreeFont();
	tft.setTextColor(BLACK);

	tft.setCursor(15, 47);
	tft.print("Time");
	tft.drawFastHLine(15, 55, 230, BLACK);
	tft.setCursor(15, 60);
	tft.print(dataEntries[0].time);
	tft.setCursor(15, 73);
	tft.print(dataEntries[1].time);
	tft.setCursor(15, 86);
	tft.print(dataEntries[2].time);
	tft.setCursor(15, 99);
	tft.print(dataEntries[3].time);
	tft.setCursor(15, 112);
	tft.print(dataEntries[4].time);
	tft.setCursor(15, 125);
	tft.print(dataEntries[5].time);
	tft.setCursor(15, 138);
	tft.print(dataEntries[6].time);
	tft.setCursor(15, 151);
	tft.print(dataEntries[7].time);
	tft.setCursor(15, 164);
	tft.print(dataEntries[8].time);
	tft.setCursor(15, 177);
	tft.print(dataEntries[9].time);

	byte xP = 80;

	tft.setCursor(xP, 47);
	tft.print("Date");
	tft.setCursor(xP, 60);
	tft.print(dataEntries[0].date);
	tft.setCursor(xP, 73);
	tft.print(dataEntries[1].date);
	tft.setCursor(xP, 86);
	tft.print(dataEntries[2].date);
	tft.setCursor(xP, 99);
	tft.print(dataEntries[3].date);
	tft.setCursor(xP, 112);
	tft.print(dataEntries[4].date);
	tft.setCursor(xP, 125);
	tft.print(dataEntries[5].date);
	tft.setCursor(xP, 138);
	tft.print(dataEntries[6].date);
	tft.setCursor(xP, 151);
	tft.print(dataEntries[7].date);
	tft.setCursor(xP, 164);
	tft.print(dataEntries[8].date);
	tft.setCursor(xP, 177);
	tft.print(dataEntries[9].date);

	xP = 155;

	tft.setCursor(xP, 47);
	tft.print("Type");
	tft.setCursor(xP, 60);
	tft.print(dataEntries[0].category);
	tft.setCursor(xP, 73);
	tft.print(dataEntries[1].category);
	tft.setCursor(xP, 86);
	tft.print(dataEntries[2].category);
	tft.setCursor(xP, 99);
	tft.print(dataEntries[3].category);
	tft.setCursor(xP, 112);
	tft.print(dataEntries[4].category);
	tft.setCursor(xP, 125);
	tft.print(dataEntries[5].category);
	tft.setCursor(xP, 138);
	tft.print(dataEntries[6].category);
	tft.setCursor(xP, 151);
	tft.print(dataEntries[7].category);
	tft.setCursor(xP, 164);
	tft.print(dataEntries[8].category);
	tft.setCursor(xP, 177);
	tft.print(dataEntries[9].category);

	xP = 195;

	tft.setCursor(xP, 47);
	tft.print("Accuracy");
	tft.setCursor(xP, 60);
	tft.print(dataEntries[0].percentage);
	tft.setCursor(xP, 73);
	tft.print(dataEntries[1].percentage);
	tft.setCursor(xP, 86);
	tft.print(dataEntries[2].percentage);
	tft.setCursor(xP, 99);
	tft.print(dataEntries[3].percentage);
	tft.setCursor(xP, 112);
	tft.print(dataEntries[4].percentage);
	tft.setCursor(xP, 125);
	tft.print(dataEntries[5].percentage);
	tft.setCursor(xP, 138);
	tft.print(dataEntries[6].percentage);
	tft.setCursor(xP, 151);
	tft.print(dataEntries[7].percentage);
	tft.setCursor(xP, 164);
	tft.print(dataEntries[8].percentage);
	tft.setCursor(xP, 177);
	tft.print(dataEntries[9].percentage);

	newDataReceived = false;

} // Close function


/*-----------------------------------------------------------------*/
// Clear any serial data

void clearSerialBuffer() {

	while (Serial.available() > 0) {
		char c = Serial.read(); // Read and discard each character
	}

} // Close function

/*-----------------------------------------------------------------*/
