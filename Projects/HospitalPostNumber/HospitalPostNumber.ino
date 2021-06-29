// Copyright 2019
// NodeMcu with 1.44 TFT ST7735 SPI display
//
// 1.44 TFT ST7735 SPI display Information
// ----> https://www.youtube.com/watch?v=9qndU3JJiKk
//    The Pin Connection:
//      LCD       ESP8266
//      ------------------
//      VCC         3.3V
//      GND         GND
//      CS          D8
//      Reset       D3(3.3V ?)
//      A0 or D/C   D4
//      SDA (MOSI)  D7
//      SCK         D5
//      LED         3.3V
//      ----------------
//
// Learn how to parse a JSON document on Arduino
// Deserialization tutorial
// https://arduinojson.org/v5/doc/decoding/

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include <SPI.h>

// For the breakout board, you can use any 2 or 3 pins.
// These pins will also work for the 1.8" TFT shield.
#define TFT_CS        15
#define TFT_RST        0 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC         2

#define DEBUG 1
// For 1.44" and 1.8" TFT with ST7735 (including HalloWing) use:
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// Muti WiFi object
ESP8266WiFiMulti wifiMulti;

// 立安診所 15*16 bitmap
//#define CHT_FONT_HEIGHT 15
//#define CHT_FONT_WIDTH 16
//const uint8_t Le_big5_15_16[] = {0x01, 0x00, 0x00, 0x80, 0x00, 0x84, 0x3f, 0xfe, 0x00, 0x00, 0x00, 0x08, 0x08, 0x18, 0x04, 0x10, 0x06, 0x20, 0x02, 0x20, 0x02, 0x40, 0x00, 0x02, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x00};
//const uint8_t An_big5_15_16[] = {0x01, 0x00, 0x00, 0x84, 0x1f, 0xfe, 0x10, 0x04, 0x21, 0x08, 0x01, 0x02, 0x7f, 0xff, 0x02, 0x10, 0x0c, 0x10, 0x03, 0x20, 0x00, 0xc0, 0x03, 0x60, 0x0c, 0x18, 0x30, 0x0c, 0x00, 0x00};
//const uint8_t Chen_big5_15_16[] = {0x20, 0x10, 0x10, 0x28, 0x00, 0x44, 0x7d, 0x83, 0x00, 0x00, 0x7c, 0x18, 0x00, 0x60, 0x7c, 0x04, 0x00, 0x18, 0x7c, 0x60, 0x44, 0x02, 0x44, 0x04, 0x7c, 0x18, 0x44, 0xe0, 0x00, 0x00};
//const uint8_t So_big5_15_16[] = {0x01, 0x02, 0x03, 0x8e, 0x3c, 0x70, 0x10, 0x40, 0x1f, 0x42, 0x11, 0x7f, 0x11, 0x44, 0x1f, 0x44, 0x11, 0x44, 0x10, 0x44, 0x20, 0x44, 0x20, 0x84, 0x41, 0x04, 0x00, 0x04, 0x00, 0x00};
//tft.drawBitmap(0,0, Le_big5_15_16, 16, 15, ST77XX_YELLOW);
//tft.drawBitmap(16,0, An_big5_15_16, 16, 15, ST77XX_YELLOW);
//tft.drawBitmap(32,0, Chen_big5_15_16, 16, 15, ST77XX_YELLOW);
//tft.drawBitmap(48,0, So_big5_15_16, 16, 15, ST77XX_YELLOW);

// 立安診所 24*24 bitmap
#define CHT_FONT_HEIGHT 24
#define CHT_FONT_WIDTH 24
const uint8_t Title_big5_24_24[4][72] = {
  { //立
    0x00, 0x38, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x00, 0x00, 0x30, 0x0c, 0x7f,
    0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0x08, 0x00, 0xe0, 0x04, 0x00, 0xc0, 0x06, 0x00,
    0xc0, 0x02, 0x01, 0x80, 0x03, 0x01, 0x80, 0x03, 0x01, 0x80, 0x01, 0x81, 0x00, 0x01, 0x81, 0x00,
    0x01, 0x83, 0x00, 0x01, 0xc2, 0x00, 0x01, 0x86, 0x00, 0x00, 0x04, 0x04, 0x00, 0x08, 0x0e, 0xff,
    0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  },
  { //安
    0x00, 0x1c, 0x00, 0x00, 0x18, 0x00, 0x20, 0x18, 0x00, 0x20, 0x18, 0x04, 0x7f, 0xff, 0xfe, 0x60,
    0x00, 0x0e, 0xe0, 0x60, 0x0c, 0xc0, 0x60, 0x18, 0x00, 0xc0, 0x00, 0x00, 0xc0, 0x00, 0x01, 0x80,
    0x06, 0xff, 0xff, 0xff, 0x01, 0x80, 0xc0, 0x03, 0x00, 0xc0, 0x03, 0x01, 0x80, 0x06, 0x01, 0x80,
    0x06, 0x03, 0x00, 0x0f, 0xc3, 0x00, 0x0c, 0x3e, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x19, 0xc0, 0x00,
    0x70, 0x70, 0x03, 0xc0, 0x1c, 0xfe, 0x00, 0x0c
  },
  { //診
    0x00, 0x01, 0xc0, 0x38, 0x01, 0x80, 0x18, 0x03, 0x40, 0x0c, 0x03, 0x40, 0x04, 0x86, 0x20, 0xff,
    0xc6, 0x30, 0x00, 0x0c, 0x18, 0x00, 0x18, 0x0f, 0x01, 0x20, 0x86, 0x7f, 0x81, 0xc0, 0x00, 0x03,
    0x00, 0x00, 0x06, 0x10, 0x7f, 0x98, 0x38, 0x00, 0x00, 0x70, 0x00, 0x00, 0xc0, 0x40, 0x81, 0x82,
    0x7f, 0xc6, 0x07, 0x61, 0x98, 0x0e, 0x61, 0x80, 0x18, 0x61, 0x80, 0x30, 0x61, 0x80, 0x60, 0x61,
    0x81, 0x80, 0x7f, 0x86, 0x00, 0x60, 0x38, 0x00
  },
  { //所
    0x00, 0x60, 0x06, 0xff, 0xf4, 0x3f, 0x00, 0x07, 0xe0, 0x00, 0x06, 0x00, 0x00, 0x06, 0x00, 0x40,
    0x46, 0x00, 0x7f, 0xe6, 0x00, 0x60, 0xe6, 0x06, 0x60, 0xc7, 0xff, 0x60, 0xc6, 0x18, 0x60, 0xc6,
    0x18, 0x60, 0xc6, 0x18, 0x7f, 0xc6, 0x18, 0x60, 0xc6, 0x18, 0x60, 0x0c, 0x18, 0x60, 0x0c, 0x18,
    0x60, 0x0c, 0x18, 0x60, 0x18, 0x18, 0x60, 0x18, 0x18, 0x40, 0x30, 0x18, 0xc0, 0x30, 0x18, 0x80,
    0x60, 0x18, 0x80, 0x80, 0x18, 0x00, 0x00, 0x18
  }
};
const uint8_t Close_big5_24_24[2][72] = {
  { //休
    0x07, 0x03, 0x80, 0x06, 0x03, 0x00, 0x06, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c, 0x03, 0x00, 0x0c,
    0x03, 0x06, 0x19, 0xff, 0xff, 0x18, 0x03, 0x00, 0x18, 0x07, 0x00, 0x30, 0x07, 0x80, 0x30, 0x0f,
    0x80, 0x58, 0x0f, 0x40, 0x58, 0x1b, 0x60, 0x98, 0x1b, 0x20, 0x18, 0x33, 0x30, 0x18, 0x33, 0x18,
    0x18, 0x63, 0x1c, 0x18, 0xc3, 0x0f, 0x19, 0x83, 0x07, 0x1a, 0x03, 0x02, 0x18, 0x03, 0x00, 0x18,
    0x03, 0x00, 0x18, 0x03, 0x00, 0x18, 0x03, 0x00
  },
  { //診
    0x00, 0x01, 0xc0, 0x38, 0x01, 0x80, 0x18, 0x03, 0x40, 0x0c, 0x03, 0x40, 0x04, 0x86, 0x20, 0xff,
    0xc6, 0x30, 0x00, 0x0c, 0x18, 0x00, 0x18, 0x0f, 0x01, 0x20, 0x86, 0x7f, 0x81, 0xc0, 0x00, 0x03,
    0x00, 0x00, 0x06, 0x10, 0x7f, 0x98, 0x38, 0x00, 0x00, 0x70, 0x00, 0x00, 0xc0, 0x40, 0x81, 0x82,
    0x7f, 0xc6, 0x07, 0x61, 0x98, 0x0e, 0x61, 0x80, 0x18, 0x61, 0x80, 0x30, 0x61, 0x80, 0x60, 0x61,
    0x81, 0x80, 0x7f, 0x86, 0x00, 0x60, 0x38, 0x00
  }
};


// Declare global variable
String InfoArray[6];//Got 6 pointers to char
boolean RefreshDisplay;
boolean SameNumber;
String DataTitleStr;
String UpdateInfoStr;
int ROpenValue;
String CurrentRTimeStr;
int TFT_WIDTH;
int TFT_HEIGHT;


void PrintTitle() {
  int X = ((TFT_WIDTH) - ((CHT_FONT_WIDTH + 1) * 4)) / 2;
  for (int i = 0; i < 4; i++) {
    tft.drawBitmap((X + ((CHT_FONT_WIDTH + 1)*i)), 0, Title_big5_24_24[i], CHT_FONT_WIDTH, CHT_FONT_HEIGHT, ST77XX_YELLOW);
  }
}

void setup() {
  // Intialize serial out:
  Serial.begin(115200);

  // Initialize 1.44" TFT:
  tft.initR(INITR_144GREENTAB); // Init ST7735R chip, green tab
  tft.fillScreen(ST77XX_BLACK);

  // Connecting WIFI
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP("Hinet D-Link WBR2200", "0912841613"); //WiFi connection 1
  wifiMulti.addAP("NoInternet_2.4G", "0912841613"); //WiFi connection 1
  wifiMulti.addAP("PPAP", "0912841613");                 //WiFi connection 2
  wifiMulti.addAP("hpguest", "");                 //WiFi connection 3
  while (wifiMulti.run() != WL_CONNECTED) {  //Wait for the WiFI connection completion
    delay(500);
    tft.setCursor(0, 30);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_BLUE);
    tft.println("WIFI Connecting....");
#if DEBUG
    Serial.println("Waiting for connection");
#endif
  }

  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 20);
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW);
  tft.println("WIFI Connect Success");

  //
  // Initial Global parameter
  //
  for (int i = 0; i < 6; i++) {
    InfoArray[i] = "--:-- 0000";
  }
  RefreshDisplay = 0;
  SameNumber = 0;
  DataTitleStr = "";
  UpdateInfoStr = "";
  ROpenValue = 0;
  CurrentRTimeStr = "";
  TFT_WIDTH = tft.width() - 1;
  TFT_HEIGHT = tft.height() - 1;
}

void loop() {

  if (wifiMulti.run()  == WL_CONNECTED) { //Check WiFi connection status
    // Here is the POST requests payload
    // payload={
    //    "PortalID":"ghyeLyQesK4",
    //    "Vcode":"86193 ",
    //    "ReqFmt":"LIST"
    //     }
    StaticJsonBuffer<500> JSONbuffer;   //Declaring static JSON buffer
    JsonObject& JSONencoder = JSONbuffer.createObject();

    JSONencoder["PortalID"] = "PortalID";
    JSONencoder["Vcode"] = "86193 ";
    JSONencoder["ReqFmt"] = "LIST ";

    char JSONmessageBuffer[500];
    JSONencoder.prettyPrintTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);

    HTTPClient http;    //Declare object of class HTTPClient

    http.begin("http://wroom.vision.com.tw/WServ/VWWL_Clinics.svc/GetWaitInfo");      //Specify request destination
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");  //Specify content-type header

    int httpCode = http.POST(JSONmessageBuffer);   //Send the request
#if DEBUG
    Serial.println(httpCode);   //Print HTTP return code
#endif

    if (httpCode == 200) {
      String payload = http.getString();   //Get the response payload
#if DEBUG
      Serial.println(payload);    //Print request response payload  //{"Code":"0000","Desc":"執行成功","Infos":[{"MTime":"最後更新：2019-01-09 12:28:18","RDate":"2019-01-09","RName":"","ROpen":1,"RTime":"0046","RVID":1,"RVID1":"","WDoct":"","WRecs":"","WSect":""}]}
      Serial.println(payload.length());
#endif
      JsonObject& JSONdecoder = JSONbuffer.parseObject(payload);

      if (JSONdecoder.success()) {
#if DEBUG
        Serial.println("payload parseObject() Successed");
#endif
        JsonObject& InfosDecoder = JSONdecoder["Infos"][0];

        ROpenValue = InfosDecoder["ROpen"];
        if (ROpenValue == 1) {
#if DEBUG
          Serial.println("Door Open");
#endif
          // Process Rdata
          String RDataStr = InfosDecoder["RDate"];
          DataTitleStr = RDataStr;
          // Process MTime
          String MTimeStr = InfosDecoder["MTime"];
          int Index = MTimeStr.indexOf(':');
          String UpdateTimeStr = MTimeStr.substring(Index - 2, Index + 3);
          // Process RValue
          String RTimeStr = InfosDecoder["RTime"];
          UpdateInfoStr = UpdateTimeStr + " " + RTimeStr;
          Serial.println("UpdateTimeStr: " + UpdateTimeStr);
          Serial.println("CurrentRTimeStr: " + CurrentRTimeStr);
          Serial.println("RTimeStr: " + RTimeStr);
          if (CurrentRTimeStr == RTimeStr) {
            Serial.println("CurrentRTimeStr == RTimeStr");
            SameNumber = 1;
          } else {
            Serial.println("CurrentRTimeStr != RTimeStr");
            SameNumber = 0;
            CurrentRTimeStr = RTimeStr;
          }
          if (UpdateInfoStr != InfoArray[0]) {
            RefreshDisplay = 1;
          }
          //Serial.println(DataTitleStr);
          Serial.println(UpdateInfoStr);
        } else {
          Serial.println("Door Close");
        } // if (ROpenValue ==1) end
      } else {
        Serial.println("payload parseObject() failed");
      } //if (!JSONdecoder.success()) end
    } else {
      Serial.println("HTTP POST Error");
    } // if (httpCode == 200) end
    http.end();  //Close connection
  } else {
    Serial.println("HTTP POST Error!");
  }

  //
  // Update Info Array Strings
  //
  Serial.println(RefreshDisplay);
  Serial.println("SameNumber " + String(SameNumber));
  Serial.println("InfoArray[0]: " + InfoArray[0]);

  if (RefreshDisplay == 1) {
    if (SameNumber == 1) {
      InfoArray[0] = UpdateInfoStr;
    } else {
      for (int i = 5; i > 0; i--) {
        InfoArray[i] = InfoArray[i - 1];
      }
      InfoArray[0] = UpdateInfoStr;
      Serial.println(UpdateInfoStr);
      Serial.println(InfoArray[0]);
    }
  }

  //
  // Display information on TFT
  //
  if (RefreshDisplay == 1) {
    RefreshDisplay = 0;
    tft.fillScreen(ST77XX_BLACK);

    PrintTitle();

    tft.setCursor(4, 26);
    tft.setTextSize(2); // size 2 = 12*16 pixels,
    tft.setTextColor(ST77XX_WHITE);
    tft.println(DataTitleStr);

    tft.drawLine(0, tft.getCursorY() + 1, TFT_WIDTH, tft.getCursorY() + 1, ST77XX_WHITE);

    tft.setCursor(4, tft.getCursorY() + 5);
    tft.setTextSize(2); // size 2 = 12*16 pixels,
    tft.setTextColor(ST77XX_GREEN);
    tft.println(InfoArray[0]);

    for  (int i = 0; i < 4; i++) {
      tft.setCursor(4, (tft.height() - 1 - (16 * (4 - i))));
      tft.setTextSize(2); // size 1 = 6*8 pixels,
      tft.setTextColor(ST77XX_RED);
      tft.println(InfoArray[i + 1]);
    }
  }

  if (ROpenValue == 0) {
    String CloseStr = "CLOSE!!";
    Serial.println(CloseStr.length());
    tft.fillScreen(ST77XX_BLACK);
    PrintTitle();
    int X = (TFT_WIDTH - ((CHT_FONT_WIDTH + 1) * 2)) / 2;
    int Y = (((TFT_HEIGHT - (CHT_FONT_HEIGHT + 1) * 2)) / 2) + (CHT_FONT_HEIGHT + 1);
    for (int i = 0; i < 2; i++) {
      tft.drawBitmap((X + ((CHT_FONT_WIDTH + 1)*i)), Y, Close_big5_24_24[i], CHT_FONT_WIDTH, CHT_FONT_HEIGHT, ST77XX_RED);
    }
  }

  delay(30000);  //Send a request every 30 seconds
}

