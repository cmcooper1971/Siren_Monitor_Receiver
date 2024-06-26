// 
// fileOperations.cpp
// 

// Main libraries

#include <FS.h>						// Files system library
#include <SD.h>						// SD Card library
#include <SPIFFS.h>					// Spiffs library

// Local declarations

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

/*---------------------------------------------------------------- */

// File name

const char* fileName = "/data.csv";			// File object to handle CSV file

// Data array

String dataReceived;						// Storing received data from Nano BLE
bleSignal dataEntries[maxEntries];			// Array to store the last 10 entries
boolean newDataReceived = false;			// Flag for each time serial data is received

/*-----------------------------------------------------------------*/

// Read File

String readFile(fs::FS& fs, const char* path) {

	Serial.printf("Reading file: %s\r\n", path);

	File file = fs.open(path);

	if (!file || file.isDirectory()) {

		outputDebugLn("- failed to open file for reading");
		return String();
	}

	String fileContent;

	while (file.available()) {

		fileContent = file.readStringUntil('\r\n');
		break;
	}

	return fileContent;

} // Close function.

/*-----------------------------------------------------------------*/

// Write file

void writeFile(fs::FS& fs, const char* path, const char* message) {

	Serial.printf("Writing file: %s\r\n", path);

	File file = fs.open(path, FILE_WRITE);

	if (!file) {

		outputDebugLn("- failed to open file for writing");
		return;
	}

	if (file.print(message)) {

		outputDebugLn("- file written");
	}

	else
	{
		outputDebugLn("- write failed");
	}

} // Close function.

/*-----------------------------------------------------------------*/

// List directory

void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {

	Serial.printf("Listing directory: %s\n", dirname);

	File root = fs.open(dirname);

	if (!root) {
		outputDebugLn("Failed to open directory");
		return;
	}

	if (!root.isDirectory()) {
		outputDebugLn("Not a directory");
		return;
	}

	File file = root.openNextFile();

	while (file) {

		if (file.isDirectory()) {
			Serial.print("  DIR : ");
			Serial.println(file.name());
			if (levels) {
				listDir(fs, file.name(), levels - 1);
			}
		}

		else {

			Serial.print("  FILE: ");
			Serial.print(file.name());
			Serial.print("  SIZE: ");
			Serial.println(file.size());
		}

		file = root.openNextFile();
	}

} // Close function

/*-----------------------------------------------------------------*/

// Create CSV file

bool createCSVFile(const char* fileName) {

	// Open the CSV file in write mode

	File file = SD.open(fileName, FILE_WRITE);

	if (file) {

		// Write header to the file

		file.println("Test,01-01-2024,00:00:00,M,100%");
		file.close();
		return true;
	}

	else {
		return false;
	}

} // Close function

/*-----------------------------------------------------------------*/

// Update CSV file

void appendFile(fs::FS& fs, const char* path, bleSignal newData) {

	Serial.printf("Appending to file: %s\r\n", path);

	// Open the file in append mode

	File file = fs.open(path, FILE_APPEND);

	if (!file) {
		outputDebugLn("Failed to open file for appending");
		return;
	}

	// Construct the message from the bleSignal struct fields

	String message = newData.title + "," + newData.date + "," + newData.time + "," + newData.category + "," + newData.percentage;

	// Append the message to the file

	if (file.println(message)) {
		
		outputDebug("Message appended = ");
		outputDebug(message);
		outputDebugLn("");
	}

	else {
		outputDebugLn("Append failed");

	}

	// Close the file

	file.close();

} // Close function

/*-----------------------------------------------------------------*/

// Parse CSV line for reading into array

bleSignal parseCSVLine(const String& line) {

	bleSignal data;
	int start = 0, end = line.indexOf(',');

	// Extract and trim title
	data.title = line.substring(start, end);
	data.title.trim();  // Clean up the title to remove any control characters and whitespace

	start = end + 1; end = line.indexOf(',', start);
	// Extract and trim date
	data.date = line.substring(start, end);
	data.date.trim();  // Also apply trim to ensure cleanliness

	start = end + 1; end = line.indexOf(',', start);
	// Extract and trim time
	data.time = line.substring(start, end);
	data.time.trim();  // Clean up time

	start = end + 1; end = line.indexOf(',', start);
	// Extract and trim category
	data.category = line.substring(start, end);
	data.category.trim();  // Clean up category

	start = end + 1;
	// Extract and trim percentage
	data.percentage = line.substring(start);
	data.percentage.trim();  // Ensure no trailing newlines or whitespace

	return data;

} // Close function

/*-----------------------------------------------------------------*/

// Convert String to CSV line

String toCSVLine(const bleSignal& data) {

	return data.title + "," + data.date + "," + data.time + "," + data.category + "," + data.percentage;

} // Close function

/*-----------------------------------------------------------------*/

// Wait for category selection

String waitForCategorySelection() {

	// Implement this function to handle user input for category selection

	// Check for touch data

	uint16_t x, y;		// variables for touch data.

	while (tft.getTouch(&x, &y)) {

		// Button one

		if ((x > BUTTON1_X) && (x < (BUTTON1_X + BUTTON1_W))) {
			if ((y > BUTTON1_Y) && (y <= (BUTTON1_Y + BUTTON1_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebugLn("");
				outputDebug("Button 1 pressed - P Returned");
				outputDebugLn("");

				return "P";

			}

		}

		// Button two

		if ((x > BUTTON2_X) && (x < (BUTTON2_X + BUTTON2_W))) {
			if ((y > BUTTON2_Y) && (y <= (BUTTON2_Y + BUTTON2_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebugLn("");
				outputDebug("Button 2 pressed - A Returned");
				outputDebugLn("");

				return "A";

			}


		}

		// Button three

		if ((x > BUTTON3_X) && (x < (BUTTON3_X + BUTTON3_W))) {
			if ((y > BUTTON3_Y) && (y <= (BUTTON3_Y + BUTTON3_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebugLn("");
				outputDebug("Button 3 hit - F Returned");
				outputDebugLn("");

				return "F";

			}

		}

		// Button four

		if ((x > BUTTON4_X) && (x < (BUTTON4_X + BUTTON4_W))) {
			if ((y > BUTTON4_Y) && (y <= (BUTTON4_Y + BUTTON4_H))) {

				playTone(buzzerP, buzzerF, buzzerD);

				outputDebugLn("");
				outputDebug("Button 4 hit - O Returned");
				outputDebugLn("");

				return "O";

			}

		}

	}

} // Close function

/*-----------------------------------------------------------------*/

// Categorise entries

void categorizeEntries(fs::FS& fs, const char* path) {

	String lineEnding = "\r\n";

	File readFile = fs.open(path, FILE_READ);

	if (!readFile) {

		Serial.println("Failed to open file for reading.");
		return;

	}

	String tempPath = String(path) + ".tmp";

	File writeFile = fs.open(tempPath.c_str(), FILE_WRITE);

	if (!writeFile) {

		outputDebugLn("Failed to open temporary file for writing.");
		readFile.close();
		return;

	}

	bool updated = false;

	while (readFile.available()) {

		String line = readFile.readStringUntil('%');

		if (!updated) {

			bleSignal data = parseCSVLine(line);

			if (data.category == "U") {

				// Print the original data
				outputDebugLn("");
				outputDebug("Title:              ");
				outputDebugLn(data.title);
				outputDebug("Date:               ");
				outputDebugLn(data.date);
				outputDebug("Time:               ");
				outputDebugLn(data.time);
				outputDebug("Category:           ");
				outputDebugLn(data.category);
				outputDebug("Percentage:         ");
				outputDebug(data.percentage);
				outputDebugLn("");

				// Print and display the original data

				drawWhiteBox();

				tft.setFreeFont(&FreeSans9pt7b);
				tft.setTextSize(1);
				tft.setTextColor(BLACK);
				tft.setCursor(13, 26);
				tft.print("Update Entry");

				tft.fillRect(39, 60, 183, 40, WHITE);
				tft.setFreeFont(&FreeSans9pt7b);
				tft.setTextSize(1);
				tft.setTextColor(DKBLUE);
				tft.setCursor(20, 88);
				tft.print("Title:");
				tft.setCursor(130, 88);
				tft.println(data.title);
				tft.setCursor(20, 108);
				tft.print("Date:");
				tft.setCursor(130, 108);
				tft.println(data.date);
				tft.setCursor(20, 128);
				tft.print("Time:");
				tft.setCursor(130, 128);
				tft.println(data.time);
				tft.setCursor(20, 148);
				tft.print("Category:");
				tft.setCursor(130, 148);
				tft.println(data.category);
				tft.setCursor(20, 168);
				tft.print("Accuracy:");
				tft.setCursor(130, 168);
				tft.print(data.percentage);
				tft.println("%");

				// Update the category
				data.category = waitForCategorySelection();

				// Print the updated data
				outputDebugLn("");
				outputDebug("Updated Title:      ");
				outputDebugLn(data.title);
				outputDebug("Updated Date:       ");
				outputDebugLn(data.date);
				outputDebug("Updated Time:       ");
				outputDebugLn(data.time);
				outputDebug("Updated Category:   ");
				outputDebugLn(data.category);
				outputDebug("Updated Accuracy: ");
				outputDebug(data.percentage);
				outputDebugLn("");
					
				line = lineEnding + toCSVLine(data);	// Convert the updated struct back to a CSV line
				updated = true;							// Mark as updated
			}
		}

		writeFile.print(line + '%');

		if (updated) {
			break;										// Exit the loop after updating the first 'U'
		}
	}

	// Copy the rest of the file if not at the end yet

	while (readFile.available()) {

		writeFile.write(readFile.read());

	}

	readFile.close();
	writeFile.close();

	// Replace the original file with the updated one

	if (updated) {

		fs.remove(path);
		fs.rename(tempPath.c_str(), path);

	}

	else {

		fs.remove(tempPath.c_str());

		// Display a message on the TFT saying no 'U' was found

		drawWhiteBox();

		tft.fillRect(39, 60, 183, 40, WHITE);
		tft.setFreeFont(&FreeSans12pt7b);
		tft.setTextSize(1);
		tft.setTextColor(DKBLUE); tft.setCursor(40, 88);
		tft.print("All Categories Set");

		delay(1500);

	}

} // Close function

/*-----------------------------------------------------------------*/

// Function to add a manual entry to the CSV file

void addManualEntry(fs::FS& fs, const char* path) {

	// Get current time and date

	char timeBuffer[20];
	time_t now = time(nullptr);
	strftime(timeBuffer, sizeof(timeBuffer), "%d-%m-%Y,%H:%M:%S", localtime(&now));

	// Construct the new entry with 'M' for manual and update the category

	String tempCat = waitForCategorySelection();

	String newEntry = "Manual," + String(timeBuffer) + ",ME-" + tempCat + ",100%";

	// Open the file in append mode

	File file = fs.open(path, FILE_APPEND);

	if (!file) {

		Serial.println("Failed to open file for appending.");
		return;

	}

	// Append the new entry

	file.println(newEntry);
	file.close();

	Serial.println("Manual entry added: " + newEntry);

} // Close function

/*-----------------------------------------------------------------*/

// Take a copy of the CSV file

bool createDataCopy(fs::FS& fs, const char* path) {

	bool createCopy = areYouSure();

	if (createCopy == true) {

		int counter = 1;
		String baseFilename = path;
		baseFilename.replace(".csv", "");  // Remove the .csv extension to append numbers
		String newFilename = baseFilename + String(counter) + ".csv";

		// Check for existing files and increment the counter to find a new filename

		while (fs.exists(newFilename.c_str())) {
			counter++;
			newFilename = baseFilename + String(counter) + ".csv";
		}

		// Open the original file and the new file

		File originalFile = fs.open(path, FILE_READ);

		if (!originalFile) {
			outputDebugLn("Failed to open the original file for reading.");
			return false;
		}

		File newFile = fs.open(newFilename.c_str(), FILE_WRITE);

		if (!newFile) {
			outputDebugLn("Failed to open the new file for writing.");
			originalFile.close();
			return false;
		}

		// Copy the content

		while (originalFile.available()) {
			newFile.write(originalFile.read());
		}

		// Close both files

		originalFile.close();
		newFile.close();

		outputDebug("File copied to: ");
		outputDebugLn(newFilename);

		return true;

	}

	else return false;

} // Close function

/*-----------------------------------------------------------------*/

// Delete last entry

void deleteLastEntry(fs::FS& fs, const char* path) {

	File readFile = fs.open(path, FILE_READ);

	if (!readFile) {
		Serial.println("Failed to open file for reading.");
		return;
	}

	String tempPath = String(path) + ".tmp";

	File writeFile = fs.open(tempPath.c_str(), FILE_WRITE);

	if (!writeFile) {
		Serial.println("Failed to open temporary file for writing.");
		readFile.close();
		return;
	}

	// Variables to track the current and previous lines

	String currentLine, lastLine;

	// Read first line outside of the loop to handle it separately

	if (readFile.available()) {
		lastLine = readFile.readStringUntil('\n');
		lastLine.trim();								// Clean up the line
	}

	// Process all remaining lines

	while (readFile.available()) {
		currentLine = readFile.readStringUntil('\n');
		currentLine.trim();								// Clean up the line

		if (!currentLine.isEmpty()) {
			writeFile.println(lastLine);				// Write the previous line to the file
			lastLine = currentLine;						// Update lastLine to current
		}
	}

	// The loop ends with lastLine not being written to writeFile, effectively deleting it

	readFile.close();
	writeFile.close();

	// Delete the original file and rename the temp file to the original file name

	if (fs.exists(path)) {
		fs.remove(path);
	
	}

	fs.rename(tempPath.c_str(), path);

} // Close function

/*-----------------------------------------------------------------*/
