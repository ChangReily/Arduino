// ArduinoJson - arduinojson.org
// Copyright Benoit Blanchon 2014-2018
// MIT License
//
// This example shows how to parse a JSON document in an HTTP response.
// It uses the Ethernet library, but can be easily adapted for Wifi.
//
//
// Learn how to parse a JSON document on Arduino
// Deserialization tutorial
// https://arduinojson.org/v5/doc/decoding/


#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

void setup() {
  Serial.begin(115200);                //Serial connection
  WiFi.begin("PPAP", "0912841613");   //WiFi connection

  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
    delay(500);
    Serial.println("Waiting for connection");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    // Here is the expected response:
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
    String payload = http.getString();                                        //Get the response payload

    //Serial.println(httpCode);   //Print HTTP return code
    //Serial.println(payload);    //Print request response payload
    //{"Code":"0000","Desc":"執行成功","Infos":[{"MTime":"最後更新：2019-01-09 12:28:18","RDate":"2019-01-09","RName":"","ROpen":1,"RTime":"0046","RVID":1,"RVID1":"","WDoct":"","WRecs":"","WSect":""}]}
    JsonObject& JSONdecoder = JSONbuffer.parseObject(payload);

    if (!JSONdecoder.success()) {
      Serial.println("payload parseObject() failed");
    } else {
      //Serial.println("payload parseObject() Successed");

      JsonObject& InfosDecoder= JSONdecoder["Infos"][0];
      String MTimeStr= InfosDecoder["MTime"];
      int Index = MTimeStr.indexOf(':');
      String UpdateTimeStr=MTimeStr.substring(Index-2);
      
      String RDaateStr= InfosDecoder["RDate"];
      String RValueStr= InfosDecoder["RTime"];
      
      int RValueNum=RValueStr.toInt();

      Serial.println(RDaateStr);
      Serial.println(UpdateTimeStr);
      Serial.println(RValueNum);
      
      
    }
    http.end();  //Close connection
  } else {
    Serial.println("Error in WiFi connection");
  }
  delay(30000);  //Send a request every 30 seconds
}
