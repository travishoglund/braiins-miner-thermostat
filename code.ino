#include <Hash.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <WebServer.h>
#include <EEPROM.h>
#include "qrcoded.h"
#include <TFT_eSPI.h>
#include <JC_Button.h>
#include <ArduinoJson.h>
#include "Bitcoin.h"
#include <Adafruit_MPL115A2.h>
#define BTN1 39 // Screen tap button
#define SDA 18
#define SCL 19

// Program Variables - do not change these
unsigned long lastTemperatureCheckInterrupt;
unsigned long lastNetworkInterrupt;
String applicationInfo[12] = {};
boolean wifiFirstPaint = true;
boolean paintScreen = true;
String screen = "init";
String qrData;
boolean firstRun = true;
unsigned long backlightCurrentMode = HIGH;

// Miner and Thermostat Settings
String minerIPAddressString = "";
IPAddress minerIPAddress;
String minerUser;
String minerPoolAddress;
float minerCurrentHashrateInTh;
int minerCurrentPowerInWatts = 0;
int minerCurrentAcceptedShares = 0;
int minerCurrentRejectedShares = 0;
int thermostatDeadbandRange = 3;
int thermostatTargetTemp = 75;
int thermostatCurrentTemp = 71;
bool minerIsMining = true;


//Network Variables - do not change these
int i = 0;
int statusCode;
const char* ssid = "default_ssid";
const char* passphrase = "default_pass";
String st;
String content;
String esid;
String epass = "";
String edeadband = "3";
String eMinerIp = "";
const int httpsPort = 443;


//Function Decalration
bool testWifi(void);
void launchWeb(void);
void setupAP(void);

// Setting up temperature reader
Adafruit_MPL115A2 mpl115a2;

// Setting up http for API Calls
WiFiClient client;
HTTPClient http;

//Establishing Local server at port 80
WebServer server(80);

// LCD Display & Clicking Settings
TFT_eSPI tft = TFT_eSPI();

// @TODO: Hookup Buttons for Thermostat Settings
Button BTNA(BTN1);

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
			*y += tft.textsize * 8;             // advance y one line
			// min/max x/y unchaged -- that waits for next 'normal' character
		}
		else if (c != '\r') {  // Normal char; ignore carriage returns
			if (/*wrap*/ false && ((*x + tft.textsize * 6) > tft.width())) { // Off right?
				*x = 0;                    // Reset x to zero,
				*y += tft.textsize * 8;         // advance y one line
			}
			int x2 = *x + tft.textsize * 6 - 1, // Lower-right pixel of char
				y2 = *y + tft.textsize * 8 - 1;
			if (x2 > *maxx) *maxx = x2;      // Track max x, y
			if (y2 > *maxy) *maxy = y2;
			if (*x < *minx) *minx = *x;      // Track min x, y
			if (*y < *miny) *miny = *y;
			*x += tft.textsize * 6;             // Advance x one char
		}
	}
}

void getTextBounds(const char *str, int16_t x, int16_t y,
	int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h) {
	uint8_t c; // Current character

	*x1 = x;
	*y1 = y;
	*w = *h = 0;

	int16_t minx = tft.width(), miny = tft.width(), maxx = -1, maxy = -1;

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
  tft.setTextColor(color, TFT_BLACK);
  tft.setCursor(x, y);                          //Set cursor to print string in centre
  tft.print(buf);                                                     //Display string
}

void printCenter(const String buf, int x, int y, long color = TFT_WHITE)                          //Function to centre the current price in the display width
{
  int16_t x1, y1;
  uint16_t w, h;
  getTextBounds(buf.c_str(), x, y, &x1, &y1, &w, &h);                     //Calculate string width
  tft.setTextColor(color, TFT_BLACK);
  tft.setCursor((x - w / 2) + (320 / 2), y);                          //Set cursor to print string in centre
  tft.print(buf);                                                     //Display string
}

void printRight(const String buf, int x, int y, long color = TFT_WHITE)                          //Function to centre the current price in the display width
{
  int16_t x1, y1;
  uint16_t w, h;
  getTextBounds(buf.c_str(), x, y, &x1, &y1, &w, &h);                     //Calculate string width
  tft.setTextColor(color, TFT_BLACK);
  tft.setCursor(320 + x - w, y);                          //Set cursor to print string in centre
  tft.print(buf);                                                     //Display string
}

/**
 * Application Functions
 */
 // Adds application info the to stack to be used easily within the application
void addApplicationInfo(String info){
  Serial.println(info);
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
        tft.fillRect(xOffset + 4 * x, yOffset + 4 * y, 4, 4, TFT_ORANGE);
      }
      else
      {
        tft.fillRect(xOffset + 4 * x, yOffset + 4 * y, 4, 4, TFT_BLACK);
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
String sendMinerCommand(String command) {

  Serial.println("Connecting to Miner");
  minerIPAddress.fromString(minerIPAddressString);
  if (client.connect(minerIPAddress, (uint16_t)4028, int32_t(1000))) {                 // Establish a connection

      if (client.connected()) {
        client.println("{\"command\": \""+command+"\"}");
      }

      while (!client.available());                // wait for response

      String response = client.readStringUntil('\n');  // read entire response
      Serial.print("[Rx] ");
      Serial.println(response);

      return response;

  }

  return "";

}

/*
 * Halt Mining - the room is too warm
 */
void stopMinerMining() {
  minerIsMining = false;
  sendMinerCommand("pause");
}

/*
 * Resume Mining - the room is too cool
 */
void startMinerMining() {
  minerIsMining = true;
  sendMinerCommand("resume");
}

/**
 * Gets the latest info from the miner and updates the application with current room temperature
 */
void updateMinerData(){

  String summaryResponse = sendMinerCommand("summary");
  String tunerResponse = sendMinerCommand("tunerstatus");
  String poolsResponse = sendMinerCommand("pools");
  StaticJsonDocument<2000> doc;

  DeserializationError error = deserializeJson(doc, summaryResponse);
  if (!error) {
    minerCurrentHashrateInTh = doc["SUMMARY"][0]["MHS 5s"].as<float>() / 1000000;
    minerCurrentAcceptedShares = doc["SUMMARY"][0]["Accepted"].as<int>();
    minerCurrentRejectedShares = doc["SUMMARY"][0]["Rejected"].as<int>();
  }

  error = deserializeJson(doc, tunerResponse);
  if (!error) {
    minerCurrentPowerInWatts = doc["TUNERSTATUS"][0]["ApproximateMinerPowerConsumption"].as<int>();
  }

  error = deserializeJson(doc, poolsResponse);
  if (!error) {
    minerUser = doc["POOLS"][0]["User"].as<String>();
    minerPoolAddress = doc["POOLS"][0]["Stratum URL"].as<String>();
  }

  // Refresh Temperature
  float pressureKPA = 0, temperatureC = 0;
  mpl115a2.getPT(&pressureKPA,&temperatureC);
  float thermostatReadTempInF = (temperatureC * 1.8 + 32);
  Serial.println(thermostatReadTempInF);
  if( thermostatReadTempInF <= (thermostatTargetTemp - thermostatDeadbandRange) ) {
    startMinerMining();
  } else if(thermostatReadTempInF >= (thermostatTargetTemp + thermostatDeadbandRange) ) {
    stopMinerMining();
  }
  thermostatCurrentTemp = (int)(thermostatReadTempInF + 0.5);

  paintScreen = true;

}

/**
 * Screens for Application
 */
void infoScreen()
{
  int cursorX = 20;
  int cursorY = 0;
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  for (int i = 0; i < 12; i++) {
    if (applicationInfo[i]) {
      cursorY = cursorY + 30;
      tft.setCursor(cursorX, cursorY);
      tft.println(applicationInfo[i]);
      delay(500);
    }
  }
}

void homeScreen(){

  clearApplicationInfo();
  int startSummaryAtY = 160;

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextSize(2);

  // Thermostat Settings
  printCenter("Miner Thermostat", 0, 12, TFT_ORANGE);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(6);
  printCenter((String)thermostatTargetTemp, 0, 46, TFT_WHITE);
  tft.setTextSize(2);
  printCenter("(Currently: " + (String)thermostatCurrentTemp + ")", 0, 110, TFT_WHITE);
  tft.setTextColor(TFT_ORANGE, TFT_BLACK);
  tft.setTextSize(2);

  // Miner Summary
  printCenter("Miner Summary", 0, startSummaryAtY, TFT_ORANGE);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(1);
  // Row 1 - Mining
  printLeft("Mining", 20, startSummaryAtY + 30, TFT_WHITE);
  printRight(minerIsMining ? "Active" : "Paused", -20, startSummaryAtY + 30, TFT_WHITE);
  // Row 1 - IP
  printLeft("Miner IP", 20, startSummaryAtY + 50, TFT_WHITE);
  printRight((String)minerIPAddressString, -20, startSummaryAtY + 50, TFT_WHITE);
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
    Serial.print("*");
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
}

void launchWeb() {
  Serial.println("");
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer();
  // Start the server
  server.begin();
  Serial.println("Server started");
}

void setupAP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      //Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("");
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
  Serial.println("Initializing_softap_for_wifi credentials_modification");
  launchWeb();
  Serial.println("over");
  tft.setCursor(40, 280);
}

void createWebServer() {
  {
    server.on("/", []() {
      IPAddress ip = WiFi.softAPIP();
      content = "<!DOCTYPE HTML>\r\n<html>";
      content = "\r\n<h1>Bitcoin Thermostat Configuration</h1>";
      content += "<p>";
      content += "</p><form method='get' action='setting'>";
      content += "<label>Wifi Network: </label><input name='ssid' type='text' length=32><br />";
      content += "<label>Wifi Password: </label><input name='ssidpass' type='text' length=64><br /><br />";
      content += "<label>Deadband Range: </label><input name='deadband' type='text' value='"+edeadband+"' length=1><br /><br />";
      content += "<label>Miner IP Address: </label><input name='minerip' type='text' length=15><br /><br />";
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
      String qminerip = server.arg("minerip");
      if (qsid.length() > 0 && qpass.length() > 0) {
        Serial.println("clearing eeprom");
        for (int i = 0; i < 112; ++i) {
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
        for (int i = 0; i < qdeadband.length(); ++i)
        {
          EEPROM.write(96 + i, qdeadband[i]);
        }
        // Write EEPROM miner ip
        for (int i = 0; i < qminerip.length(); ++i)
        {
          EEPROM.write(97 + i, qminerip[i]);
        }

        EEPROM.commit();

        content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
        statusCode = 200;
        ESP.restart();
      } else {
        content = "{\"Error\":\"404 not found\"}";
        statusCode = 404;
        Serial.println("Sending 404");
      }
      server.sendHeader("Access-Control-Allow-Origin", "*");
      server.send(statusCode, "application/json", content);

    });
  }
}

/**
 * Converts the character array to all uppercase
 */
void to_upper(char* arr) {
  for (size_t i = 0; i < strlen(arr); i++) {
    if (arr[i] >= 'a' && arr[i] <= 'z') {
      arr[i] = arr[i] - 'a' + 'A';
    }
  }
}

/**
 * Encrypts the payload
 */
int xor_encrypt(uint8_t* output,
                size_t outlen,
                uint8_t* key,
                size_t keylen,
                uint8_t* nonce,
                size_t nonce_len,
                uint64_t pin,
                uint64_t amount_in_cents) {
  // check we have space for all the data:
  // <variant_byte><len|nonce><len|payload:{pin}{amount}><hmac>
  if (outlen <
      2 + nonce_len + 1 + lenVarInt(pin) + 1 + lenVarInt(amount_in_cents) + 8) {
    return 0;
  }

  int cur = 0;
  output[cur] = 1;  // variant: XOR encryption
  cur++;

  // nonce_len | nonce
  output[cur] = nonce_len;
  cur++;
  memcpy(output + cur, nonce, nonce_len);
  cur += nonce_len;

  // payload, unxored first - <pin><currency byte><amount>
  int payload_len = lenVarInt(pin) + 1 + lenVarInt(amount_in_cents);
  output[cur] = (uint8_t)payload_len;
  cur++;
  uint8_t* payload = output + cur;  // pointer to the start of the payload
  cur += writeVarInt(pin, output + cur, outlen - cur);              // pin code
  cur += writeVarInt(amount_in_cents, output + cur, outlen - cur);  // amount
  cur++;

  // xor it with round key
  uint8_t hmacresult[32];
  SHA256 h;
  h.beginHMAC(key, keylen);
  h.write((uint8_t*)"Round secret:", 13);
  h.write(nonce, nonce_len);
  h.endHMAC(hmacresult);
  for (int i = 0; i < payload_len; i++) {
    payload[i] = payload[i] ^ hmacresult[i];
  }

  // add hmac to authenticate
  h.beginHMAC(key, keylen);
  h.write((uint8_t*)"Data:", 5);
  h.write(output, cur);
  h.endHMAC(hmacresult);
  memcpy(output + cur, hmacresult, 8);
  cur += 8;

  // return number of bytes written to the output
  return cur;
}

void setup() {

  // Initialize SDA/SCL via WIRE
  Wire.begin(SDA, SCL);
  if (! mpl115a2.begin()) {
    addApplicationInfo("Sensor not found! Check wiring");
    while (1);
  }

  // Setup Screen Button
  BTNA.begin();

  // Setup Screen
  tft.init();
  tft.setRotation(0);
  tft.invertDisplay(false);
  tft.fillScreen(TFT_BLACK);

  // Button
  pinMode(0,INPUT_PULLUP);
  pinMode(35,INPUT_PULLUP);

  // Draw Logo
  tft.drawBitmap(96, 208, bitcoinLogo, 128, 64, TFT_ORANGE);

  // Setup Serial
  Serial.begin(9600); //Initialising Serial Monitor
  addApplicationInfo("Booting...");

  // Setup Timeouts for Interrupts
  lastNetworkInterrupt = millis();
  lastTemperatureCheckInterrupt = millis();

  // Start Logging Boot Sequence
  addApplicationInfo("Disconnecting wifi");
  WiFi.disconnect();

  // Initializing EEPROM
  EEPROM.begin(512);
  delay(10);
  pinMode(15, INPUT);

  // Read eeprom ssid
  addApplicationInfo("Reading SSID info");
  // Read eeprom ssid
  for (int i = 0; i < 32; ++i) {
    esid += char(EEPROM.read(i));
  }

  // Read eeprom password
  for (int i = 32; i < 96; ++i) {
    epass += char(EEPROM.read(i));
  }

  // Read eeprom deadband range
  String readDeadband = "";
  for (int i = 96; i < 97; ++i) {
    readDeadband = char(EEPROM.read(i));
    if(readDeadband.toInt() > 0) {
      thermostatDeadbandRange = readDeadband.toInt();
    }
  }

  // Read eeprom miner ip
  for (int i = 97; i < 112; ++i) {
    eMinerIp += char(EEPROM.read(i));
  }
  minerIPAddressString = eMinerIp.c_str();

  // Print EEPROM settings for debugging
  Serial.println("EEPROM: SSID: " + esid + ", SSID-PASS: " + epass + ", DeadbandRange: " + thermostatDeadbandRange + ", Miner IP: " + minerIPAddressString);

  // Try to connect to wifi from stored credentials
  addApplicationInfo("Connecting to Wifi");
  WiFi.begin(esid.c_str(), epass.c_str());
}

void loop() {

  // Wifi is connected properly
  if ((WiFi.status() == WL_CONNECTED))
  {
    // Refresh Bitcoin Price every 15 seconds
    if(screen != "init" && (millis() - lastTemperatureCheckInterrupt > 15000)) {
      updateMinerData();
      lastTemperatureCheckInterrupt = millis();
    }

    // Screen Logic
    if(paintScreen) {
      paintScreen = false;
      if(screen == "init") {
        addApplicationInfo("Connected to:");
        addApplicationInfo(esid);
        infoScreen();
        lastTemperatureCheckInterrupt = millis();
        updateMinerData();
        delay(3000);
        screen = "home";
        paintScreen = true;
      } else if(screen == "home") {
        homeScreen();
      } else {
        infoScreen();
      }
    }
  }

  // Setup Access Point due to no internet connection
  if(firstRun) {
    if (testWifi() && (digitalRead(15) != 1))
    {} else {
      // Setup HotSpot and show user that Connection is ready
      // Connect to Azteco ATM Wifi Network to configure
      firstRun = false;
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