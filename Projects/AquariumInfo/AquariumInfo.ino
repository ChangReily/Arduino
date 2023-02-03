#include <ArduinoJson.h>  // Arduino Jason v5
#include <ESP8266WiFi.h>
#include <EspMQTTClient.h>
#include <NTPClient.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


EspMQTTClient client("NoInternet_2.4G",  // Wi-Fi AP name
                     "0912841613",       // Wi-Fi Password
                     "192.168.0.98",     // MQTT Broker server ip
                     "",                 // Broker user name; Can be omitted if not needed
                     "",                 // Broker password; Can be omitted if not needed
                     "AquariumTankInfo",     // Client name that uniquely identify your device
                     1883                // The MQTT port, default to 1883. this line can be omitted
);


WiFiUDP ntpUDP;
NTPClient gtimeClient(ntpUDP);

float TemperatureValue = 0;
float PhValue = 0;
float TdsValue = 0;
int LightRelay = 0;
int Co2Relay = 0;

int DisplayItemIdx = 0;

int NtpReady = 0;
int ReceivedMessage = 0;

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

// ----------------------------------------------------------------------------
// void setup()
//   function setup
// ----------------------------------------------------------------------------
void setup(void) {
  Serial.begin(115200);
  delay(3000);
  Serial.println("....................");

  // Starts the underlying UDP client
  gtimeClient.begin();
  // Changes the time offset to UTC+8 (8 hr = 28800 secs)
  gtimeClient.setTimeOffset(28800);

  // Enable debugging messages sent to serial output
  client.enableDebuggingMessages();
  lcd.init();                      // initialize the lcd 
 
  // Print a message to the LCD.
  lcd.backlight();

}

// ----------------------------------------------------------------------------
// void loop()
//   function loop
// ----------------------------------------------------------------------------
void loop() {
  DisplayLoop();
  client.loop();
}

// ----------------------------------------------------------------------------
// void onConnectionEstablished()
//   This function is called once everything is connected (Wifi and MQTT)
//   WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
// ----------------------------------------------------------------------------
void onConnectionEstablished() {
  // Update NTP time and print it
  while (gtimeClient.getEpochTime() < 1640995200) {
    Serial.println("NTP Time less than 2022-01-01");
    delay(500);
    if (!gtimeClient.forceUpdate()) {
      gtimeClient.update();
    }
  }
  NtpReady=1;
  Serial.print("MQTT Connect NTP Time: ");
  Serial.println(gtimeClient.getFormattedTime());

  // Subscribe to "Aquarium/SendCmd" and declare callback function
  client.subscribe("Aquarium/SendData", ProcessCmd);

}

// ----------------------------------------------------------------------------
// void ProcessCmd(const String& topicStr, const String& message)
//  3Callback function for Subscribe to "Aquarium/SendCmd"
// ----------------------------------------------------------------------------
void ProcessCmd(const String& topicStr, const String& message) {
  // Serial.println("message received from " + topicStr + ": " + message);
  if (message.equals("NULL")) {
    return;
  }
  ReceivedMessage= 1;
  const int BufferSize = JSON_OBJECT_SIZE(5)+50;
  StaticJsonBuffer<BufferSize> JSONbuffer;  // Declaring static JSON buffer
  JsonObject& JSONdecoder = JSONbuffer.parseObject(message);
  JSONdecoder.prettyPrintTo(Serial);
  Serial.println(" ");
  
  if (JSONdecoder.containsKey("temp")) {
    TemperatureValue = JSONdecoder["temp"];
    Serial.println("temp: " + String(TemperatureValue));
  }
  if (JSONdecoder.containsKey("ph")) {
    PhValue = JSONdecoder["ph"];
    Serial.println("ph: " + String(PhValue));
  }
  if (JSONdecoder.containsKey("tds")) {
    TdsValue = JSONdecoder["tds"];
    Serial.println("tds: " + String(TdsValue));
  }
  if (JSONdecoder.containsKey("light")) {
    LightRelay = JSONdecoder["light"];
    Serial.println("light: " + String(LightRelay));
  }
  if (JSONdecoder.containsKey("co2")) {
    Co2Relay = JSONdecoder["co2"];
    Serial.println("co2: " + String(Co2Relay));
  }

}


void DisplayLoop() {
  unsigned long CurrentMillis = millis();
  static unsigned long Loop1secPreviousMillis = 0;
  bool mCo2yStatus = true;
  float phvalue;
  if (CurrentMillis - Loop1secPreviousMillis >= 1000) {
    Loop1secPreviousMillis = CurrentMillis;
    // Serial.println(gtimeClient.getFormattedTime() + " --- Loop_1_Sec()");
    gtimeClient.update();


    if (NtpReady == 0){
      Serial.println(gtimeClient.getFormattedTime() + " Wait for Internet..");
      return;
    } else if (ReceivedMessage == 0){
      Serial.println(gtimeClient.getFormattedTime() + " Wait for Data..");
      return;
    } 
    lcd.setCursor(0, 0);
    lcd.print("PH:"+String(PhValue));
    
    Serial.println("DisplayItemIdx:" + String(DisplayItemIdx));
    switch (DisplayItemIdx){
      case 0:
        lcd.setCursor(0, 1);
        lcd.print("TDS:"+String(TdsValue));
        break;
      case 3:
        lcd.setCursor(0, 1);
        lcd.print("Temp:"+String(TemperatureValue));
        break;
    }


    DisplayItemIdx = DisplayItemIdx + 1;
    if (DisplayItemIdx == 6){
      DisplayItemIdx = 0;
    }
  }
}
