#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <EEPROM.h>
#include "qrcoded.h"
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define SDA 18
#define SCL 19
#define LGFX_WT32_SC01
#define LGFX_USE_V1
#define LGFX_AUTODETECT

#include <LovyanGFX.hpp>

// Program Variables - do not change these
unsigned long lastTemperatureCheckInterrupt;
unsigned long lastMinerUpdateInterrupt;
unsigned long lastNetworkInterrupt;
unsigned long lastTouchInterrupt;
String applicationInfo[12] = {};
boolean wifiFirstPaint = true;
boolean paintScreen = true;
String screen = "init";
String qrData;
boolean firstRun = true;
static int32_t x,y;

// Miner and Thermostat Settings
IPAddress minerIpAddresses[5];
String minerUser;
String minerPoolAddress;
String minerIpsFragmented;
float minerCurrentHashrateInTh;
int minerCurrentPowerInWatts = 0;
int minerCurrentAcceptedShares = 0;
int minerCurrentRejectedShares = 0;
int thermostatDeadbandRange = 2;
int thermostatTargetTemp = 75;
int thermostatCurrentTemp = 71;
bool minerIsMining = true;
int minersConnected = 0;
bool debug = false;
bool forceConfig = false;


//Network Variables - do not change these
int i = 0;
int statusCode;
const char* ssid = "default_ssid";
const char* passphrase = "default_pass";
String st;
String content;
String esid;
String epass = "";
String edeadband = "2";
String eMinerIp1 = "";
String eMinerIp2 = "";
String eMinerIp3 = "";
String eMinerIp4 = "";
String eMinerIp5 = "";
const int httpsPort = 443;


//Function Declaration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

// Setting up http for API Calls
WiFiClient client;
HTTPClient http;

//Establishing Local server at port 80
WebServer server(80);

// LCD Display
static LGFX lcd;

// Temperature Sensor
OneWire oneWire(2);
DallasTemperature tempSensor(&oneWire);

const unsigned char bitcoinLogo [] PROGMEM =                                  // 'Bitcoin Logo', 128x64px
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xc0, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x03, 0xe7, 0xc0, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x03, 0xe7, 0xc0, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x02, 0x24, 0x40, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x02, 0x24, 0x40, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x02, 0x24, 0x40, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x02, 0x24, 0x40, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x02, 0x3c, 0x40, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0x03, 0xfe, 0x3c, 0x7c, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x1f, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0e, 0x03, 0x00, 0x00, 0x01, 0xc0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0c, 0x03, 0x00, 0x00, 0x00, 0xc0, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x03, 0xf0, 0x3f, 0x80, 0x60, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0xf0, 0x3f, 0xe0, 0x20, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x18, 0x30, 0x70, 0x30, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x18, 0x30, 0x10, 0x30, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x18, 0x30, 0x10, 0x30, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x18, 0x30, 0x10, 0x30, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x18, 0x30, 0x30, 0x20, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x18, 0x30, 0xe0, 0x60, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x18, 0x3f, 0xc0, 0xc0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x18, 0x00, 0x01, 0xc0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x18, 0x00, 0x00, 0xf0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x18, 0x00, 0x00, 0x38, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x18, 0x3f, 0xe0, 0x18, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x18, 0x30, 0xf8, 0x0c, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x18, 0x30, 0x1c, 0x0c, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x18, 0x30, 0x0c, 0x0c, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x18, 0x30, 0x04, 0x0c, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x18, 0x30, 0x0c, 0x0c, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x18, 0x30, 0x0c, 0x0c, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x38, 0x00, 0x18, 0x30, 0x38, 0x0c, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x18, 0x01, 0xf0, 0x3f, 0xf0, 0x08, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x01, 0xe0, 0x1f, 0x00, 0x18, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1c, 0x01, 0x00, 0x00, 0x00, 0x30, 0x03, 0x80, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x0e, 0x03, 0x00, 0x00, 0x00, 0xe0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x06, 0x03, 0x00, 0x00, 0x0f, 0xc0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x07, 0x03, 0xfe, 0x3c, 0x7e, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x03, 0x80, 0x02, 0x3c, 0x40, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x02, 0x24, 0x40, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x02, 0x24, 0x40, 0x00, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x02, 0x24, 0x40, 0x00, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x02, 0x24, 0x40, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x38, 0x03, 0xe7, 0xc0, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x03, 0xe7, 0xc0, 0x07, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xf0, 0x00, 0x00, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x03, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xc0, 0x3f, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/**
 * Helper Functions
 */
 void charBounds(char c, int16_t *x, int16_t *y,
	int16_t *minx, int16_t *miny, int16_t *maxx, int16_t *maxy) {

	if (!true) {}
	else { // Default font

		if (c == '\n') {                     // Newline?
			*x = 0;                        // Reset x to zero,
			*y += lcd.getTextSizeX() * 8;             // advance y one line
			// min/max x/y unchaged -- that waits for next 'normal' character
		}
		else if (c != '\r') {  // Normal char; ignore carriage returns
			if (/*wrap*/ false && ((*x + lcd.getTextSizeX() * 6) > lcd.width())) { // Off right?
				*x = 0;                    // Reset x to zero,
				*y += lcd.getTextSizeX() * 8;         // advance y one line
			}
			int x2 = *x + lcd.getTextSizeX() * 6 - 1, // Lower-right pixel of char
				y2 = *y + lcd.getTextSizeX() * 8 - 1;
			if (x2 > *maxx) *maxx = x2;      // Track max x, y
			if (y2 > *maxy) *maxy = y2;
			if (*x < *minx) *minx = *x;      // Track min x, y
			if (*y < *miny) *miny = *y;
			*x += lcd.getTextSizeX() * 6;             // Advance x one char
		}
	}
}

void getTextBounds(const char *str, int16_t x, int16_t y,
	int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
	uint8_t c; // Current character

	*x1 = x;
	*y1 = y;
	*w = *h = 0;

	int16_t minx = lcd.width(), miny = lcd.width(), maxx = -1, maxy = -1;

	while ((c = *str++))
		charBounds(c, &x, &y, &minx, &miny, &maxx, &maxy);

	if (maxx >= minx) {
		*x1 = minx;
		*w = maxx - minx + 1;
	}
	if (maxy >= miny) {
		*y1 = miny;
		*h = maxy - miny + 1;
	}
}

// Prints centered text
void printLeft(const String buf, int x, int y, long color = TFT_WHITE)                          //Function to centre the current price in the display width
{
  int16_t x1, y1;
  lcd.setTextColor(color, TFT_BLACK);
  lcd.setCursor(x, y);                          //Set cursor to print string in centre
  lcd.print(buf);                                                     //Display string
}

void printCenter(const String buf, int x, int y, long color = TFT_WHITE)                          //Function to centre the current price in the display width
{
  int16_t x1, y1;
  uint16_t w, h;
  getTextBounds(buf.c_str(), x, y, &x1, &y1, &w, &h);                     //Calculate string width
  lcd.setTextColor(color, TFT_BLACK);
  lcd.setCursor((x - w / 2) + (320 / 2), y);                          //Set cursor to print string in centre
  lcd.print(buf);                                                     //Display string
}

void printRight(const String buf, int x, int y, long color = TFT_WHITE)                          //Function to centre the current price in the display width
{
  int16_t x1, y1;
  uint16_t w, h;
  getTextBounds(buf.c_str(), x, y, &x1, &y1, &w, &h);                     //Calculate string width
  lcd.setTextColor(color, TFT_BLACK);
  lcd.setCursor(320 + x - w, y);                          //Set cursor to print string in centre
  lcd.print(buf);                                                     //Display string
}

/**
 * Application Functions
 */
 // Adds application info the to stack to be used easily within the application
void addApplicationInfo(String info){
  if(debug) Serial.println(info);
  for (int i = 0; i < 12; i++) {
    if (applicationInfo[i] == NULL) {
      applicationInfo[i] = info;
      return;
    }
  }
}

void resetToHomeScreen() {
  clearApplicationInfo();
  screen = "home";
  lastTemperatureCheckInterrupt = millis();
}

void showQrCode(String qrData, int xOffset, int yOffset) {
  const char *qrDataChar = qrData.c_str();
  QRCode qrcoded;
  uint8_t qrcodeData[qrcode_getBufferSize(20)];
  qrcode_initText(&qrcoded, qrcodeData, 6, 0, qrDataChar);

  for (uint8_t y = 0; y < qrcoded.size; y++)
  {
    for (uint8_t x = 0; x < qrcoded.size; x++)
    {
      if (qrcode_getModule(&qrcoded, x, y))
      {
        lcd.fillRect(xOffset + 4 * x, yOffset + 4 * y, 4, 4, TFT_ORANGE);
      }
      else
      {
        lcd.fillRect(xOffset + 4 * x, yOffset + 4 * y, 4, 4, TFT_BLACK);
      }
    }
  }
}

// Clears application stack messages
void clearApplicationInfo() {
  memset(applicationInfo, '\0', sizeof(applicationInfo));
}

/**
 * Miner Connection
 */
String sendMinerCommand(String command, IPAddress minerIp) {

  if(debug) ("Connecting to Miner: " + minerIp.toString());
  if (client.connect(minerIp, (uint16_t)4028, int32_t(1000))) {                 // Establish a connection

      if (client.connected()) {
        client.println("{\"command\": \""+command+"\"}");
      }

      while (!client.available());                // wait for response

      String response = client.readStringUntil('\n');  // read entire response
      if(debug) Serial.print("[Rx] ");
      if(debug) Serial.println(response);

      return response;

  }

  return "";

}

/*
 * Halt Mining - the room is too warm
 */
void stopMinersMining() {
  if(minerIsMining != false) paintScreen = true;
  minerIsMining = false;
  for (int i = 0; i < 5; i++) {
    if(minerIpAddresses[i].toString()) {
      sendMinerCommand("pause", minerIpAddresses[i]);
    }
  }
}

/*
 * Resume Mining - the room is too cool
 */
void startMinersMining() {
  if(minerIsMining != true) paintScreen = true;
  minerIsMining = true;
  for (int i = 0; i < 5; i++) {
    if(minerIpAddresses[i].toString()) {
      sendMinerCommand("resume", minerIpAddresses[i]);
    }
  }
}

/*
 * Checks the room temperature and pauses/starts mining
 */
void checkRoomTemperature() {
  lastTemperatureCheckInterrupt = millis();

  // Refresh Temperature
  tempSensor.requestTemperatures();
  float tempCelsius = tempSensor.getTempCByIndex(0);
  float tempFahrenheit = tempCelsius * 9 / 5 + 32;

  if(debug) Serial.println(tempFahrenheit);
  if( tempFahrenheit <= (thermostatTargetTemp - thermostatDeadbandRange) ) {
    startMinersMining();
  } else if(tempFahrenheit >= (thermostatTargetTemp + thermostatDeadbandRange) ) {
    stopMinersMining();
  }

  int currentTemperatureReading = (int)(tempFahrenheit + 0.5);
  if(currentTemperatureReading != thermostatCurrentTemp && screen != "init") {
    thermostatCurrentTemp = currentTemperatureReading;
    lcd.setTextSize(2);
    printCenter(" (Currently: " + (String)thermostatCurrentTemp + ") ", 0, 110, TFT_WHITE);
  }

}

/**
 * Gets the latest info from the miner
 */
void updateMinerData(){

  lastMinerUpdateInterrupt = millis();
  minersConnected = 0;
  minerCurrentPowerInWatts = 0;
  minerCurrentHashrateInTh = 0;
  minerCurrentAcceptedShares = 0;
  minerCurrentRejectedShares = 0;


  for (int i = 0; i < 5; i++) {
    if(minerIpAddresses[i].toString()) {
      String summaryResponse = sendMinerCommand("summary", minerIpAddresses[i]);
      String tunerResponse = sendMinerCommand("tunerstatus", minerIpAddresses[i]);
      StaticJsonDocument<2000> doc;

      DeserializationError error = deserializeJson(doc, summaryResponse);
      if (!error) {
        minersConnected = minersConnected + 1;
        minerCurrentHashrateInTh = minerCurrentHashrateInTh + ( doc["SUMMARY"][0]["MHS 5s"].as<float>() / 1000000 );
        minerCurrentAcceptedShares = minerCurrentAcceptedShares + doc["SUMMARY"][0]["Accepted"].as<int>();
        minerCurrentRejectedShares = minerCurrentRejectedShares + doc["SUMMARY"][0]["Rejected"].as<int>();
      }

      error = deserializeJson(doc, tunerResponse);
      if (!error) {
        minerCurrentPowerInWatts = minerCurrentPowerInWatts + doc["TUNERSTATUS"][0]["ApproximateMinerPowerConsumption"].as<int>();
      }
    }
  }

  // Connect to the first miner only for pool information to show on Thermostat
  String poolsResponse = sendMinerCommand("pools", minerIpAddresses[0]);
  StaticJsonDocument<2000> doc;
  DeserializationError error = deserializeJson(doc, poolsResponse);
  if (!error) {
    minerUser = doc["POOLS"][0]["User"].as<String>();
    minerPoolAddress = doc["POOLS"][0]["Stratum URL"].as<String>();
  }

  paintScreen = true;

}

/**
 * Screens for Application
 */
void infoScreen()
{
  int cursorX = 20;
  int cursorY = 0;
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextSize(2);
  for (int i = 0; i < 12; i++) {
    if (applicationInfo[i]) {
      cursorY = cursorY + 30;
      lcd.setCursor(cursorX, cursorY);
      lcd.println(applicationInfo[i]);
      delay(500);
    }
  }
}

void homeScreen(){

  clearApplicationInfo();
  int startSummaryAtY = 160;

  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  lcd.setTextSize(2);

  // Thermostat Settings
  printCenter("Miner Thermostat", 0, 12, TFT_ORANGE);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextSize(6);
  printCenter((String)thermostatTargetTemp, 0, 46, TFT_WHITE);
  printCenter("-", -70, 46, TFT_WHITE);
  printCenter("+", 90, 46, TFT_WHITE);
  lcd.setTextSize(2);
  printCenter(" (Currently: " + (String)thermostatCurrentTemp + ") ", 0, 110, TFT_WHITE);
  lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  lcd.setTextSize(2);

  // Miner Summary
  printCenter("Miner Summary", 0, startSummaryAtY, TFT_ORANGE);
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextSize(1);
  // Row 1 - Mining
  printLeft("Mining", 20, startSummaryAtY + 30, TFT_WHITE);
  printRight(minerIsMining ? "Active" : "Paused", -20, startSummaryAtY + 30, TFT_WHITE);
  // Row 1 - IP
  printLeft("Miner IP(s)", 20, startSummaryAtY + 50, TFT_WHITE);
  printRight((String)minerIpsFragmented, -20, startSummaryAtY + 50, TFT_WHITE);
  // Row 2 - Deadband
  printLeft("Deadband", 20, startSummaryAtY + 70, TFT_WHITE);
  printRight("(+-) " + (String)thermostatDeadbandRange + " degrees", -20, startSummaryAtY + 70, TFT_WHITE);
  // Row 3 - Hashrate
  printLeft("Hashrate", 20, startSummaryAtY + 90, TFT_WHITE);
  printRight((String)minerCurrentHashrateInTh + " TH/s", -20, startSummaryAtY + 90, TFT_WHITE);
  // Row 4 - Power
  printLeft("Power", 20, startSummaryAtY + 110, TFT_WHITE);
  printRight((String)minerCurrentPowerInWatts + " watts", -20, startSummaryAtY + 110, TFT_WHITE);
  // Row 5 - Shares
  printLeft("Shares", 20, startSummaryAtY + 130, TFT_WHITE);
  printRight(String(minerCurrentAcceptedShares) + " shares", -20, startSummaryAtY + 130, TFT_WHITE);
  // Row 6 - Rejected Shares
  printLeft("Rejected", 20, startSummaryAtY + 150, TFT_WHITE);
  printRight(String(minerCurrentRejectedShares) + " shares", -20, startSummaryAtY + 150, TFT_WHITE);
  // Row 7 - User
  printLeft("User", 20, startSummaryAtY + 170, TFT_WHITE);
  printRight((String)minerUser, -20, startSummaryAtY + 170, TFT_WHITE);
  // Row 7 - Pool
  printLeft("Pool", 20, startSummaryAtY + 190, TFT_WHITE);
  printRight((String)minerPoolAddress, -20, startSummaryAtY + 190, TFT_WHITE);

  // Miner Connection & Reset Button
  lcd.drawCircle(300, 460, 10, (minersConnected > 0 ? TFT_GREEN : TFT_RED));
  lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  lcd.setTextSize(1);
  lcd.setCursor(298, 456);
  lcd.print(minersConnected);
  lcd.setCursor(20, 460);
  lcd.print("Reset");

}


/**
 * Network Functions for testing connectability
 */
bool testWifi(void) {
  int c = 0;
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED)
    {
      return true;
    }
    delay(500);
    if(debug) Serial.print("*");
    c++;
  }
  if(debug) Serial.println("");
  if(debug) Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb() {
  if(debug) Serial.println("");
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.print("Local IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("SoftAP IP: ");
    Serial.println(WiFi.softAPIP());
  }
  createWebServer();
  // Start the server
  server.begin();
  if(debug) Serial.println("Server started");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  if(debug) Serial.println("scan done");
  if (n == 0)
    if(debug) Serial.println("no networks found");
  else
  {
    if(debug) Serial.print(n);
    if(debug) Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      if(debug) {
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(WiFi.SSID(i));
        Serial.print(" (");
        Serial.print(WiFi.RSSI(i));
        Serial.print(")");
      }
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  if(debug) Serial.println("");
  st = "<ol>";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);

    st += ")";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.softAP("BitThermo", "");
  if(debug) Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  if(debug) Serial.println("over");
  lcd.setCursor(40, 280);
}

void createWebServer() {
  {
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      content = "<!DOCTYPE HTML>\r\n<html>";
      content = "\r\n<h1>Bitcoin Thermostat Configuration</h1>";
      content += "<p>";
      content += "</p><form method='get' action='setting'>";
      content += "<label>Wifi Network: </label><input name='ssid' type='text' length=32 value='"+esid+"'><br />";
      content += "<label>Wifi Password: </label><input name='ssidpass' type='text' length=64 value='"+epass+"'><br /><br />";
      content += "<label>Deadband Range: </label><input name='deadband' type='text' value='"+edeadband+"' length=1><br /><br />";
      content += "<label>Miner #1 IP Address: </label><input name='minerip1' type='text' value='"+eMinerIp1+"'length=15><br /><br />";
      content += "<label>Miner #2 IP Address: </label><input name='minerip2' type='text' value='"+eMinerIp2+"'length=15><br /><br />";
      content += "<label>Miner #3 IP Address: </label><input name='minerip3' type='text' value='"+eMinerIp3+"'length=15><br /><br />";
      content += "<label>Miner #4 IP Address: </label><input name='minerip4' type='text' value='"+eMinerIp4+"'length=15><br /><br />";
      content += "<label>Miner #5 IP Address: </label><input name='minerip5' type='text' value='"+eMinerIp5+"'length=15><br /><br />";
      content += "<input type='submit'></form>";
      content += "<style>body{background: #000; color: #FFF; font-family: Arial;} input[type=submit]{background: #000; border: 1px solid #f2a900; color: #f2a900; padding: 8px 16px;} label {display: block;} input[type=text]{background: #000; border: 1px solid #f2a900; color: #f2a900; margin-bottom: 10px; height: 30px; line-height: 30px; width: 100%; max-width: 300px; }</style>";
      content += "</html>";
      server.send(200, "text/html", content);
    });
    server.on("/scan", []() {
      //setupAP();
      IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);

      content = "<!DOCTYPE HTML>\r\n<html>go back";
      server.send(200, "text/html", content);
    });

    server.on("/setting", []() {
      String qsid = server.arg("ssid");
      String qpass = server.arg("ssidpass");
      String qdeadband = server.arg("deadband");
      String qminerip1 = server.arg("minerip1");
      String qminerip2 = server.arg("minerip2");
      String qminerip3 = server.arg("minerip3");
      String qminerip4 = server.arg("minerip4");
      String qminerip5 = server.arg("minerip5");
      if (qsid.length() > 0 && qpass.length() > 0) {
        if(debug) Serial.println("clearing eeprom");
        for (int i = 0; i < 185; ++i) {
          EEPROM.write(i, 0);
        }
        // Write EEPROM ssid
        for (int i = 0; i < qsid.length(); ++i)
        {
          EEPROM.write(i, qsid[i]);
        }
        // Write EEPROM ssid password
        for (int i = 0; i < qpass.length(); ++i)
        {
          EEPROM.write(32 + i, qpass[i]);
        }
        // Write EEPROM deadband
        EEPROM.write(96, qdeadband[0]);

        // Write EEPROM reset
        EEPROM.write(97, 0);

        // Write EEPROM miner ips
        for (int i = 0; i < qminerip1.length(); ++i)
        {
          EEPROM.write(98 + i, qminerip1[i]);
        }
        for (int i = 0; i < qminerip2.length(); ++i)
        {
          EEPROM.write(113 + i, qminerip2[i]);
        }
        for (int i = 0; i < qminerip3.length(); ++i)
        {
          EEPROM.write(128 + i, qminerip3[i]);
        }
        for (int i = 0; i < qminerip4.length(); ++i)
        {
          EEPROM.write(143 + i, qminerip4[i]);
        }
        for (int i = 0; i < qminerip5.length(); ++i)
        {
          EEPROM.write(168 + i, qminerip5[i]);
        }

        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.restart();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        if(debug) Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}

void setup() {

  // Initialize temperature sensor
  tempSensor.begin();

  // Setup Screen
  lcd.init();
  lcd.setRotation(0);
  lcd.fillScreen(TFT_BLACK);

  // Draw Logo
  lcd.drawBitmap(96, 208, bitcoinLogo, 128, 64, TFT_ORANGE);

  // Setup Serial
  Serial.begin(9600); //Initialising Serial Monitor
  addApplicationInfo("Booting...");

  // Setup Timeouts for Interrupts
  lastNetworkInterrupt = millis();
  lastTemperatureCheckInterrupt = millis();
  lastTouchInterrupt = millis();

  // Start Logging Boot Sequence
  addApplicationInfo("Disconnecting wifi");
  WiFi.disconnect();

  // Initializing EEPROM
  EEPROM.begin(512);
  delay(10);
  pinMode(15, INPUT);

  addApplicationInfo("Reading SSID info");

  // Read eeprom ssid
  for (int i = 0; i < 32; ++i) {
    if(EEPROM.read(i) != 255 && EEPROM.read(i) != 0) {
      esid += char(EEPROM.read(i));
    }
  }

  // Read eeprom password
  for (int i = 32; i < 96; ++i) {
    if(EEPROM.read(i) != 255 && EEPROM.read(i) != 0) {
      epass += char(EEPROM.read(i));
    }
  }

  // Read eeprom deadband range
  String readDeadband = "";
  for (int i = 96; i < 97; ++i) {
    readDeadband = char(EEPROM.read(i));
    if(readDeadband.toInt() > 0) {
      thermostatDeadbandRange = readDeadband.toInt();
    }
  }

  // Read eeprom force config
  if(EEPROM.read(97) == 1) {
    forceConfig = true;
  }

  // Read eeprom miner ips
  for (int i = 98; i < 113; ++i) {
    if(EEPROM.read(i) != 255 && EEPROM.read(i) != 0) {
      eMinerIp1 += char(EEPROM.read(i));
    }
  }
  for (int i = 113; i < 128; ++i) {
    if(EEPROM.read(i) != 255 && EEPROM.read(i) != 0) {
      eMinerIp2 += char(EEPROM.read(i));
    }
  }
  for (int i = 128; i < 143; ++i) {
    if(EEPROM.read(i) != 255 && EEPROM.read(i) != 0) {
      eMinerIp3 += char(EEPROM.read(i));
    }
  }
  for (int i = 143; i < 168; ++i) {
    if(EEPROM.read(i) != 255 && EEPROM.read(i) != 0) {
      eMinerIp4 += char(EEPROM.read(i));
    }
  }
  for (int i = 168; i < 183; ++i) {
    if(EEPROM.read(i) != 255 && EEPROM.read(i) != 0) {
      eMinerIp5 += char(EEPROM.read(i));
    }
  }

  minerIpAddresses[0].fromString(eMinerIp1.c_str());
  minerIpAddresses[1].fromString(eMinerIp2.c_str());
  minerIpAddresses[2].fromString(eMinerIp3.c_str());
  minerIpAddresses[3].fromString(eMinerIp4.c_str());
  minerIpAddresses[4].fromString(eMinerIp5.c_str());

  // Print EEPROM settings for debugging
  if(debug) {
    String forceConfigPrint = (forceConfig ? "true" : "false");
    Serial.println("EEPROM: SSID: " + esid);
    Serial.println("EEPROM: SSID-PASS: " + epass);
    Serial.println("EEPROM: Deadband Range: " + thermostatDeadbandRange);
    Serial.println("EEPROM: Miner IP(1): " + eMinerIp1);
    Serial.println("EEPROM: Miner IP(2): " + eMinerIp2);
    Serial.println("EEPROM: Miner IP(3): " + eMinerIp3);
    Serial.println("EEPROM: Miner IP(4): " + eMinerIp4);
    Serial.println("EEPROM: Miner IP(5): " + eMinerIp5);
    Serial.println("EEPROM: Force Config: " + forceConfigPrint);
  }

  minerIpsFragmented = minerIpAddresses[0].toString();
  if(eMinerIp2) minerIpsFragmented = minerIpsFragmented + " /" + minerIpAddresses[1][3];
  if(eMinerIp3) minerIpsFragmented = minerIpsFragmented + "/" + minerIpAddresses[2][3];
  if(eMinerIp4) minerIpsFragmented = minerIpsFragmented + "/" + minerIpAddresses[3][3];
  if(eMinerIp5) minerIpsFragmented = minerIpsFragmented + "/" + minerIpAddresses[4][3];

  // Try to connect to wifi from stored credentials
  addApplicationInfo("Connecting to Wifi");
  WiFi.begin(esid.c_str(), epass.c_str());
}

void loop() {

  // Wifi is connected properly
  if (forceConfig == false && (WiFi.status() == WL_CONNECTED))
  {
    // Refresh temperature status every 5 seconds
    if(screen != "init" && (millis() - lastTemperatureCheckInterrupt > 3000)) {
      checkRoomTemperature();
    }

    if(screen != "init" && (millis() - lastMinerUpdateInterrupt > 30000)) {
      updateMinerData();
    }

    // Screen Logic
    if(paintScreen) {
      paintScreen = false;
      if(screen == "init") {
        addApplicationInfo("Connected to:");
        addApplicationInfo(esid);
        infoScreen();
        checkRoomTemperature();
        updateMinerData();
        delay(5000);
        screen = "home";
        paintScreen = true;
      } else if(screen == "home") {
        homeScreen();
      } else {
        infoScreen();
      }
    }

    // Temperature Adjustment
    if (lcd.getTouch(&x, &y)) {
        // Adjust temp down
        if(x > 45 && x < 95 && y > 45 && y < 95) {
          if( millis() - lastTouchInterrupt > 500 ) {
            lastTouchInterrupt = millis();
            thermostatTargetTemp--;
            lcd.setTextColor(TFT_WHITE, TFT_BLACK);
            lcd.setTextSize(6);
            printCenter((String)thermostatTargetTemp, 0, 46, TFT_WHITE);
          }
        }
        // Adjust temp up
        if(x > 225 && x < 275 && y > 45 && y < 95) {
          if(millis() - lastTouchInterrupt > 500) {
            lastTouchInterrupt = millis();
            thermostatTargetTemp++;
            lcd.setTextColor(TFT_WHITE, TFT_BLACK);
            lcd.setTextSize(6);
            printCenter((String)thermostatTargetTemp, 0, 46, TFT_WHITE);
          }
        }
        // Enter Config Mode
        if(x >= 0 && x <= 90 && y >= 460 && y <= 480) {
          EEPROM.write(97, 1);
          EEPROM.commit();
          ESP.restart();
        }
      }
  }

  // Setup Access Point due to no internet connection
  if(firstRun) {
    if (forceConfig == false && testWifi() && (digitalRead(15) != 1))
    {} else {
      // Setup HotSpot and show user that Connection is ready
      firstRun = false;
      delay(50);
      clearApplicationInfo();
      addApplicationInfo("No Wifi, Hotspot on");
      addApplicationInfo("Connect to BitThermo");
      addApplicationInfo("Set wifi ssid at:");
      addApplicationInfo("192.168.4.1");
      infoScreen();
      showQrCode("http://192.168.4.1", 20, 200);
      launchWeb();
      setupAP();// Setup HotSpot
    }
  }

  // Waiting for connection details through Access Point
  if ((WiFi.status() != WL_CONNECTED))
  {
    if((millis() - lastNetworkInterrupt > 5000)) {
      lastNetworkInterrupt = millis();
      Serial.print(".");
      server.handleClient();
    }
  }
}