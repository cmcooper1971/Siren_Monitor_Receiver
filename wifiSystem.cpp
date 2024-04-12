// 
// 
// 

// Main libraries

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

// Local declarations

#include "wifiSystem.h"
#include "global.h"
#include "fileOperations.h"
#include "parseData.h"
#include "screenLayout.h"
#include "icons.h"
#include "drawBitmap.h"
#include "mainDisplay.h"
#include "Free_Fonts.h"

// Debug serial prints

#define DEBUG 1

#if DEBUG==1
#define outputDebug(x); Serial.print(x);
#define outputDebugLn(x); Serial.println(x);
#else
#define outputDebug(x); 
#define outputDebugLn(x); 
#endif

/*---------------------------------------------------------------- */

// WiFi Configuration

boolean wiFiYN = false;				// WiFi reset
boolean apMode = false;

// Search for parameter in HTTP POST request

const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "subnet";
const char* PARAM_INPUT_5 = "gateway";
const char* PARAM_INPUT_6 = "dns";

// Variables to save values from HTML form

String ssid;
String pass;
String ip;
String subnet;
String gateway;
String dns;

// File paths to save input values permanently

const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* subnetPath = "/subnet.txt";
const char* gatewayPath = "/gateway.txt";
const char* dnsPath = "/dns.txt";

// Network variables

IPAddress localIP;
IPAddress Gateway;
IPAddress Subnet;
IPAddress dns1;

// Web Server configuration

AsyncWebServer server(80);			// Create AsyncWebServer object on port 80
AsyncEventSource events("/events");	// Create an Event Source on /events

// Json Variable to hold sensor readings

DynamicJsonDocument readings(1024);

// Timer variables (check wifi).

unsigned long wiFiR = 0;					// WiFi retry (wiFiR) to attempt connecting to the last known WiFi if connection lost
unsigned long wiFiRI = 60000;				// WiFi retry Interval (wiFiRI) used to retry connecting to the last known WiFi if connection lost
volatile bool disWiFi = false;				// Used in the sensor interrupt function to disable WiFi
volatile bool disWiFiF = false;				// Used in the sensor interrupt function to disable WiFi
unsigned long previousMillis = 0;			// Used in the WiFI Init function
const long interval = 10000;				// Interval to wait for Wi-Fi connection (milliseconds)

/*---------------------------------------------------------------- */

void  checkWiFiReset(boolean& wiFiYN) {

// Check if WiFi is to be reset

if (wiFiYN == false) {

	ssid = "blank";
	pass = "blank";
	ip = "blank";
	subnet = "blank";
	gateway = "blank";
	dns = "blank";

	writeFile(SPIFFS, ssidPath, ssid.c_str());
	writeFile(SPIFFS, passPath, pass.c_str());
	writeFile(SPIFFS, ipPath, ip.c_str());
	writeFile(SPIFFS, subnetPath, subnet.c_str());
	writeFile(SPIFFS, gatewayPath, gateway.c_str());
	writeFile(SPIFFS, dnsPath, dns.c_str());

}

} // Close function

/*---------------------------------------------------------------- */

// WiFi title page

void wiFiTitle() {

	// WiFi title screen

	tft.setFreeFont(&FreeSans9pt7b);
	tft.setTextSize(1);
	tft.setTextColor(BLACK);
	tft.setCursor(13, 26);
	tft.println("Setting up WiFi");
	tft.setFreeFont();

	tft.setTextColor(BLACK);
	tft.setFreeFont();
	tft.setCursor(23, 50);
	tft.print("WiFi Status: ");
	tft.setCursor(23, 65);
	tft.print("SSID: ");
	tft.setCursor(23, 80);
	tft.print("IP Address: ");
	tft.setCursor(23, 95);
	tft.print("DNS Address: ");
	tft.setCursor(23, 110);
	tft.print("Gateway Address: ");
	tft.setCursor(23, 125);
	tft.print("Signal Strenght: ");
	tft.setCursor(23, 140);
	tft.print("Time Server: ");

} // Close function

/*-----------------------------------------------------------------*/

// Initialize WiFi

bool initWiFi() {

	wiFiTitle();

	outputDebugLn("");

	ssid = readFile(SPIFFS, ssidPath);
	pass = readFile(SPIFFS, passPath);
	ip = readFile(SPIFFS, ipPath);
	subnet = readFile(SPIFFS, subnetPath);
	gateway = readFile(SPIFFS, gatewayPath);
	dns = readFile(SPIFFS, dnsPath);

	outputDebugLn();
	outputDebugLn(ssid);
	outputDebugLn(pass);
	outputDebugLn(ip);
	outputDebugLn(subnet);
	outputDebugLn(gateway);
	outputDebugLn(dns);
	outputDebugLn();

	// If ESP32 inits successfully in station mode, recolour WiFi to red.

	drawBitmap(tft, WIFI_ICON_Y, WIFI_ICON_X, wiFiRed, WIFI_ICON_W, WIFI_ICON_H);

	// Check if settings are available to connect to WiFi.

	if (ssid == "" || ip == "") {
		outputDebugLn("Undefined SSID or IP address.");
		return false;
	}

	WiFi.mode(WIFI_STA);
	localIP.fromString(ip.c_str());
	Gateway.fromString(gateway.c_str());
	Subnet.fromString(subnet.c_str());
	dns1.fromString(dns.c_str());

	if (!WiFi.config(localIP, Gateway, Subnet, dns1)) {
		outputDebugLn("STA Failed to configure");
		return false;
	}

	WiFi.begin(ssid.c_str(), pass.c_str());
	outputDebugLn("Connecting to WiFi...");

	// If ESP32 inits successfully in station mode, recolour WiFi to amber.

	drawBitmap(tft, WIFI_ICON_Y, WIFI_ICON_X, wiFiAmber, WIFI_ICON_W, WIFI_ICON_H);

	unsigned long currentMillis = millis();
	previousMillis = currentMillis;

	while (WiFi.status() != WL_CONNECTED) {
		currentMillis = millis();
		if (currentMillis - previousMillis >= interval) {
			outputDebugLn("Failed to connect.");
			// If ESP32 fails to connect, recolour WiFi to red.
			drawBitmap(tft, WIFI_ICON_Y, WIFI_ICON_X, wiFiRed, WIFI_ICON_W, WIFI_ICON_H);
			drawWhiteBox();
			return false;
		}

	}

	outputDebugLn(WiFi.localIP());
	outputDebugLn("");
	outputDebug("RRSI: ");
	outputDebugLn(WiFi.RSSI());

	// Update TFT with settings.

	tft.setTextColor(BLACK);
	tft.setFreeFont();
	tft.setCursor(23, 50);
	tft.print("WiFi Status: ");
	tft.setTextColor(DKBLUE);
	tft.setCursor(150, 50);
	tft.print(WiFi.status());
	tft.println("");

	tft.setCursor(23, 65);
	tft.setTextColor(BLACK);
	tft.print("SSID: ");
	tft.setTextColor(DKBLUE);
	tft.setCursor(150, 65);
	tft.print(WiFi.SSID());
	tft.println("");

	tft.setCursor(23, 80);
	tft.setTextColor(BLACK);
	tft.print("IP Address: ");
	tft.setTextColor(DKBLUE);
	tft.setCursor(150, 80);
	tft.print(WiFi.localIP());
	tft.println("");

	tft.setCursor(23, 95);
	tft.setTextColor(BLACK);
	tft.print("DNS Address: ");
	tft.setTextColor(DKBLUE);
	tft.setCursor(150, 95);
	tft.print(WiFi.dnsIP());
	tft.println("");

	tft.setCursor(23, 110);
	tft.setTextColor(BLACK);
	tft.print("Gateway Address: ");
	tft.setTextColor(DKBLUE);
	tft.setCursor(150, 110);
	tft.print(WiFi.gatewayIP());
	tft.println("");

	tft.setCursor(23, 125);
	tft.setTextColor(BLACK);
	tft.print("Signal Strenght: ");
	tft.setTextColor(DKBLUE);
	tft.setCursor(150, 125);
	tft.print(WiFi.RSSI());
	tft.println("");

	tft.setCursor(23, 140);
	tft.setTextColor(BLACK);
	tft.print("Time Server: ");
	tft.setTextColor(DKBLUE);
	//tft.setCursor(150, 140);
	//tft.print(ntpServer);

	// If ESP32 inits successfully in station mode, recolour WiFi to green.

	drawBitmap(tft, WIFI_ICON_Y, WIFI_ICON_X, wiFiGreen, WIFI_ICON_W, WIFI_ICON_H);

	// Update message to advise unit is starting.

	tft.setFreeFont(&FreeSans9pt7b);
	tft.setTextSize(1);
	tft.setTextColor(BLACK);
	tft.println("");
	tft.setCursor(20, 175);
	tft.print("Unit is starting...");
	tft.setFreeFont();

	screenR = 1;

	delay(3000);	// Wait a moment.

	return true;

} // Close function.

/*-----------------------------------------------------------------*/

// Initialize WiFi

void initialiseWiFi() {

	if (initWiFi()) {

		// Handle the Web Server in Station Mode and route for root / web page

		server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {

			request->send(SPIFFS, "/index.html", "text/html");
			});

		server.serveStatic("/", SPIFFS, "/");

		// Request for the latest data readings

		server.on("/readings", HTTP_GET, [](AsyncWebServerRequest* request) {

			String json = getJSONReadings();
			request->send(200, "application/json", json);
			json = String();
			});

		events.onConnect([](AsyncEventSourceClient* client) {

			if (client->lastId()) {
				Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
			}
			});

		server.addHandler(&events);

		outputDebugLn("");
		outputDebugLn("Web Server Events Started...");
		outputDebugLn("");


		server.begin();

		outputDebugLn("");
		outputDebugLn("Web Server Started...");
		outputDebugLn("");

	}

	else

	{
		apMode = true;	// Set variable to be true so void loop is by passed and doesnt run until false

		// WiFi title page

		wiFiTitle();

		// Initialize the ESP32 in Access Point mode, recolour to WiFI red

		drawBitmap(tft, WIFI_ICON_Y, WIFI_ICON_X, wiFiRed, WIFI_ICON_W, WIFI_ICON_H);
		delay(1000);

		// Set Access Point

		outputDebugLn("Setting AP (Access Point)");

		// NULL sets an open Access Point

		WiFi.softAP("WIFI-MANAGER", NULL);

		IPAddress IP = WiFi.softAPIP();
		outputDebug("AP IP address: ");
		outputDebugLn(IP);

		// Web Server Root URL For WiFi Manager Web Page

		server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
			request->send(SPIFFS, "/wifimanager.html", "text/html");
			});

		server.serveStatic("/", SPIFFS, "/");

		// Get the parameters submited on the form

		server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {
			int params = request->params();
			for (int i = 0; i < params; i++) {
				AsyncWebParameter* p = request->getParam(i);
				if (p->isPost()) {
					// HTTP POST ssid value
					if (p->name() == PARAM_INPUT_1) {
						ssid = p->value().c_str();
						outputDebug("SSID set to: ");
						outputDebugLn(ssid);
						// Write file to save value
						writeFile(SPIFFS, ssidPath, ssid.c_str());
					}
					// HTTP POST pass value
					if (p->name() == PARAM_INPUT_2) {
						pass = p->value().c_str();
						outputDebug("Password set to: ");
						outputDebugLn(pass);
						// Write file to save value
						writeFile(SPIFFS, passPath, pass.c_str());
					}
					// HTTP POST ip value
					if (p->name() == PARAM_INPUT_3) {
						ip = p->value().c_str();
						outputDebug("IP Address set to: ");
						outputDebugLn(ip);
						// Write file to save value
						writeFile(SPIFFS, ipPath, ip.c_str());
					}
					// HTTP POST ip value
					if (p->name() == PARAM_INPUT_4) {
						subnet = p->value().c_str();
						outputDebug("Subnet Address: ");
						outputDebugLn(subnet);
						// Write file to save value
						writeFile(SPIFFS, subnetPath, subnet.c_str());
					}
					// HTTP POST ip value
					if (p->name() == PARAM_INPUT_5) {
						gateway = p->value().c_str();
						outputDebug("Gateway set to: ");
						outputDebugLn(gateway);
						// Write file to save value
						writeFile(SPIFFS, gatewayPath, gateway.c_str());
					}
					// HTTP POST ip value
					if (p->name() == PARAM_INPUT_6) {
						dns = p->value().c_str();
						outputDebug("DNS Address set to: ");
						outputDebugLn(dns);
						// Write file to save value
						writeFile(SPIFFS, dnsPath, dns.c_str());
					}
					//Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
				}
			}

			request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
			delay(3000);

			// After saving the parameters, restart the ESP32

			ESP.restart();
			});

		server.begin();

		tft.fillRect(39, 60, 183, 109, RED);
		tft.drawRect(38, 59, 185, 111, WHITE);
		tft.drawRect(37, 58, 187, 113, WHITE);
		tft.setFreeFont(&FreeSans9pt7b);
		tft.setTextSize(1);
		tft.setTextColor(WHITE); tft.setCursor(50, 78);
		tft.print("Access Point Mode");

		tft.setFreeFont();
		tft.setTextColor(WHITE);
		tft.setCursor(50, 90);
		tft.print("Could not connect to WiFi");
		tft.setCursor(50, 106);
		tft.print("1) Using your mobile phone");
		tft.setCursor(50, 118);
		tft.print("2) Connect to WiFI Manager");
		tft.setCursor(50, 130);
		tft.print("3) Browse to 192.168.4.1");
		tft.setCursor(50, 142);
		tft.print("4) Enter network settings");
		tft.setCursor(50, 154);
		tft.print("5) Unit will then restart");

		unsigned long previousMillis = millis();
		unsigned long interval = 120000;

		while (1) {

			// Hold from starting loop while in AP mode

			unsigned long currentMillis = millis();

			// Restart after 2 minutes in case of failed reconnection with correc WiFi details

			if (currentMillis - previousMillis >= interval) {

				ESP.restart();

			}

		}

	}

} // Close function

/*-----------------------------------------------------------------*/

// Return JSON String from sensor Readings

String getJSONReadings() {

	// Get siren activity

		DynamicJsonDocument doc(1024); // Adjust size as needed
		JsonArray readings = doc.createNestedArray("readings");

		for (int i = 0; i < 10; ++i) {
			JsonObject entry = readings.createNestedObject();
			entry["title"] = dataEntries[i].title;
			entry["date"] = dataEntries[i].date;
			entry["time"] = dataEntries[i].time;
			entry["category"] = dataEntries[i].category;
			entry["percentage"] = dataEntries[i].percentage;
		}

		String jsonString;
		serializeJson(doc, jsonString);
		return jsonString;

}  // Close function.

/*-----------------------------------------------------------------*/

// Update webserver with latest events

void updateWebServer() {

	String message = getJSONReadings();

	outputDebugLn("");
	outputDebug("New Readings: ");
	outputDebug(message);
	outputDebugLn("");

	events.send(message.c_str(), "new_readings", millis());

} // Close function

/*-----------------------------------------------------------------*/