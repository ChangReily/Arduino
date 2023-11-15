#include <Adafruit_ADS1X15.h>
#include <ArduinoJson.h>  // Arduino Jason v5
#include <ArduinoSort.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <EspMQTTClient.h>
#include <FS.h>
#include <NTPClient.h>
#include <OneWire.h>

EspMQTTClient client("NoInternet_2.4G",  // Wi-Fi AP name
                     "0912841613",       // Wi-Fi Password
                     "192.168.0.98",     // MQTT Broker server ip
                     "",                 // Broker user name; Can be omitted if not needed
                     "",                 // Broker password; Can be omitted if not needed
                     "AquariumTank",     // Client name that uniquely identify your device
                     1883                // The MQTT port, default to 1883. this line can be omitted
);

const char* MqttDebugTopic = "Aquarium/SerialMessage";

WiFiUDP ntpUDP;
NTPClient gtimeClient(ntpUDP);
Adafruit_ADS1115 gAds1115;

// PIN mapping define
#define LIGHT_RELAY_PIN 14          // D5 /GPIO14
// #define LIGHT_SWITCH_PIN 13         // D7 /GPIO13
#define CO2_RELAY_PIN 12            // D6 /GPIO12
// #define CO2_SWITCH_PIN 15           // D8 /GPIO15
#define TEMPERATURE_IN_WATER_PIN 2  // D4 /GPIO2
// GPIO6.GPIO7.GPIO8.GPIO9.GPIO10.GPIO11 are used for internal SPI rom.

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(TEMPERATURE_IN_WATER_PIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature TemperatureInWater(&oneWire);

// PH related value
float gPhValue;   // PH
float gPhSlope;   // PHValue = slope*voltage+Offset;
float gPhOffset;  // PHValue = slope*voltage+Offset;
float gPh4Voltage;
float gPh7Voltage;
float gPhLowLimit;   // PH
float gPhHighLimit;   // PH

// const float gTargetPh = 6.7;

// PH Array
#define PH_ARRAY_LENGTH 12
float gPhArray[PH_ARRAY_LENGTH];  // Store the average value of the sensor feedback
int gPhArrayIndex = 0;

// TDS related value
float gTdsVoltage;
float gCalibrateTdsTarget;
float gTdsValue = 0;
float gTdsKValue = 1;
float gTdsFactor = 0.5;

// TDS Array
#define TDS_ARRAY_LENGTH 12
float gTdsArray[TDS_ARRAY_LENGTH];  // Store the average value of the sensor feedback
int gTdsArrayIndex = 0;

// Temperature Value
float gTemperatureValue;

// Temperature Array
#define TEMPERATURE_ARRAY_LENGTH 12
float gTemperatureArray[TEMPERATURE_ARRAY_LENGTH];  // Store the average value of the sensor feedback
int gTemperatureArrayIndex = 0;

// Aquarium Control State
enum Control_State {
  NormalMode,
  EnterPhCalibrateMode,
  Ph4CalibrateMode,
  Ph7CalibrateMode,
  PhCalculateMode,
  EnterTdsCalibrateMode,
  TdsCalibrateMode,
  TdsCalculateMode
};
// Aquarium Control State
Control_State gState = NormalMode;

// Relay time calculate
#define CAL_TIME_SECS(Hours, Minutes, Seconds) (Hours * 60 * 60 + Minutes * 60 + Seconds)
// Relay web Control State
enum RELAY_CONTROL_STATE { 
  FORCE_ON, 
  FORCE_OFF, 
  AUTO 
};

// Light on/off time
int gLightOnHour = 14;
int gLightOnMin = 20;
int gLightOffHour = 20;
int gLightOffMin = 30;

// Light relay control
bool gLightRelay = false;
RELAY_CONTROL_STATE gLightForceControl = AUTO;

// Co2 on/off time
int gCo2OnHour = 14;
int gCo2OnMin = 20;
int gCo2OffHour = 20;
int gCo2OffMin = 30;

// Co2 relay control
bool gCo2Relay = false;
RELAY_CONTROL_STATE gCo2ForceControl = AUTO;

// Reset function
void (*resetFunc)(void) = 0;  // declare reset function at address 0

// ----------------------------------------------------------------------------
// void setup()
//   function setup
// ----------------------------------------------------------------------------
void setup(void) {
  Serial.begin(115200);
  delay(3000);
  Serial.println("....................");

  // Access internal ROM
  SPIFFS.begin();

  // Load PH config data from internal ROM
  if (!LoadConfig()) {
    Serial.println("Failed to load config");
    gPhSlope = -5.85330;
    gPhOffset = 16.06050;

    gTdsKValue = 1;

    gLightOnHour = 14;
    gLightOnMin = 30;
    gLightOffHour = 20;
    gLightOffMin = 30;

    gCo2OnHour = 14;
    gCo2OnMin = 30;
    gCo2OffHour = 20;
    gCo2OffMin = 30;

    gPhLowLimit = 6.5;   // PH
    gPhHighLimit = 6.8;   // PH

    SaveConfig();
  }

  // Starts the underlying UDP client
  gtimeClient.begin();
  // Changes the time offset to UTC+8 (8 hr = 28800 secs)
  gtimeClient.setTimeOffset(28800);

  // Enable debugging messages sent to serial output
  client.enableDebuggingMessages();

  // Start up the Temperature
  TemperatureInWater.begin();

  // Set up Ligh control GPIO pin
  pinMode(LIGHT_RELAY_PIN, OUTPUT);
  digitalWrite(LIGHT_RELAY_PIN, HIGH); 
  // pinMode(LIGHT_SWITCH_PIN, INPUT_PULLUP);

  // Set up CO2 control GPIO pin
  pinMode(CO2_RELAY_PIN, OUTPUT);
  digitalWrite(CO2_RELAY_PIN, HIGH);
  // pinMode(CO2_SWITCH_PIN, INPUT_PULLUP);

  // gAds1115.begin(0x48);
  if (!gAds1115.begin(0x48)) {
    Serial.println("Failed to initialize ADS1115.");
    while (1);
  }
}

// ----------------------------------------------------------------------------
// void loop()
//   function loop
// ----------------------------------------------------------------------------
void loop() {
  if (!client.isConnected()) {
  }

  if (client.isConnected()) {
    switch (gState) {
      case NormalMode:
        NormalLoop();
        break;
      case EnterPhCalibrateMode:
        gPhArrayIndex = 0;
        break;
      case Ph4CalibrateMode:
      case Ph7CalibrateMode:
        PhCalibrateLoop();
        break;
      case PhCalculateMode:
        PhCalculate();
        break;
      case EnterTdsCalibrateMode:
        gTdsArrayIndex = 0;
        break;
      case TdsCalibrateMode:
        TdsCalibrateLoop();
        break;
      case TdsCalculateMode:
        TdsCalculate();
        break;

      default:
        // statements
        break;
    }
  }
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

  Serial.print("MQTT Connect NTP Time: ");
  Serial.println(gtimeClient.getFormattedTime());

  // Subscribe to "Aquarium/SendCmd" and declare callback function
  client.subscribe("Aquarium/SendCmd", ProcessSendcmd);
  client.subscribe("Aquarium/SetLightTimeCmd", ProcessSetLightTimeCmd);
  client.subscribe("Aquarium/ControlLightCmd", ProcessControlLightCmd);
  client.subscribe("Aquarium/SetCo2TimeCmd", ProcessSetCo2TimeCmd);
  client.subscribe("Aquarium/SetPhLimitCmd", ProcessSetPhLimitCmd);
  client.subscribe("Aquarium/ControlCo2Cmd", ProcessControlCo2Cmd);
  client.subscribe("Aquarium/TdsCalibrateCmd", ProcessTdsCalibrateCmd);

  client.publish(MqttDebugTopic, gtimeClient.getFormattedTime());

}

// ----------------------------------------------------------------------------
// bool LoadConfig()
//   Load the PH Slope and Offset from ESP8266 internal ROM by FS.h (SPIFFS.h)
// ----------------------------------------------------------------------------
// {
//   "gPhSlope": "2.9999999999",
//   "gPhOffset": "2.9999999999",
//   "gTdsKValue": "2.9999999999",
//   "gLightOnHour": "23",
//   "gLightOnMin": "59",
//   "gLightOffHour": "23",
//   "gLightOffMin": "59",
//   "gCo2OnHour": "23",
//   "gCo2OnMin": "59",
//   "gCo2OffHour": "23",
//   "gCo2OffMin": "50",
//   "gPhLowLimit": "6.50",
//   "gPhHighLimit": "6.70"
// }

bool LoadConfig() {
  File ConfigFile = SPIFFS.open("/config.json", "r");
  if (!ConfigFile) {
    Serial.println("Failed to open config file");
    return false;
  }

  size_t size = ConfigFile.size();
  if (size > 1024) {
    Serial.println("Config file size is too large");
    return false;
  }

  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);

  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use ConfigFile.readString instead.
  ConfigFile.readBytes(buf.get(), size);
  ConfigFile.close();

  // https://arduinojson.org/v5/assistant/
  // https://arduinojson.org/v5/faq/how-to-determine-the-buffer-size/
  const int BufferSize = JSON_OBJECT_SIZE(13) + 300;
  StaticJsonBuffer<BufferSize> JsonBuffer;
  JsonObject& ConfigJson = JsonBuffer.parseObject(buf.get());

  if (!ConfigJson.success()) {
    Serial.println("Failed to parse config file");
    return false;
  }

  gPhSlope = ConfigJson["gPhSlope"];
  gPhOffset = ConfigJson["gPhOffset"];

  gTdsKValue = ConfigJson["gTdsKValue"];

  gLightOnHour = ConfigJson["gLightOnHour"];
  gLightOnMin = ConfigJson["gLightOnMin"];
  gLightOffHour = ConfigJson["gLightOffHour"];
  gLightOffMin = ConfigJson["gLightOffMin"];

  gCo2OnHour = ConfigJson["gCo2OnHour"];
  gCo2OnMin = ConfigJson["gCo2OnMin"];
  gCo2OffHour = ConfigJson["gCo2OffHour"];
  gCo2OffMin = ConfigJson["gCo2OffMin"];

  gPhLowLimit = ConfigJson["gPhLowLimit"];
  gPhHighLimit = ConfigJson["gPhHighLimit"];

  Serial.println("Load Config - ");
  Serial.print(" [PH slope]  ");
  Serial.println(gPhSlope, 5);
  Serial.print(" [PH offset] ");
  Serial.println(gPhOffset, 5);
  Serial.println("");
  Serial.print(" [TDS K Value] ");
  Serial.println(gTdsKValue, 5);
  Serial.println("");
  Serial.println(" [Light ON]  " + String(gLightOnHour) + ":" + String(gLightOnMin));
  Serial.println(" [Light OFF] " + String(gLightOffHour) + ":" + String(gLightOffMin));
  Serial.println("");
  Serial.println(" [Co2 ON]  " + String(gCo2OnHour) + ":" + String(gCo2OnMin));
  Serial.println(" [Co2 OFF] " + String(gCo2OffHour) + ":" + String(gCo2OffMin));
  Serial.println("");
  Serial.println(" [PH Limit]  " + String(gPhLowLimit) + "~" + String(gPhHighLimit));

  return true;
}

// ----------------------------------------------------------------------------
// bool SaveConfig()
//   Save the PH Slope and Offset to ESP8266 internal ROM by FS.h (SPIFFS.h)
// ----------------------------------------------------------------------------
bool SaveConfig() {
  // https://arduinojson.org/v5/assistant/
  // https://arduinojson.org/v5/faq/how-to-determine-the-buffer-size/
  const int BufferSize = JSON_OBJECT_SIZE(13);
  StaticJsonBuffer<BufferSize> JsonBuffer;

  JsonObject& ConfigJson = JsonBuffer.createObject();
  ConfigJson["gPhSlope"] = gPhSlope;
  ConfigJson["gPhOffset"] = gPhOffset;

  ConfigJson["gTdsKValue"] = gTdsKValue;

  ConfigJson["gLightOnHour"] = gLightOnHour;
  ConfigJson["gLightOnMin"] = gLightOnMin;
  ConfigJson["gLightOffHour"] = gLightOffHour;
  ConfigJson["gLightOffMin"] = gLightOffMin;

  ConfigJson["gCo2OnHour"] = gCo2OnHour;
  ConfigJson["gCo2OnMin"] = gCo2OnMin;
  ConfigJson["gCo2OffHour"] = gCo2OffHour;
  ConfigJson["gCo2OffMin"] = gCo2OffMin;
  ConfigJson["gPhLowLimit"] = gPhLowLimit;
  ConfigJson["gPhHighLimit"] = gPhHighLimit;

  ConfigJson.prettyPrintTo(Serial);
  Serial.println("");

  File ConfigFile = SPIFFS.open("/config.json", "w");
  if (!ConfigFile) {
    Serial.println("Failed to open config file for writing");
    return false;
  }

  // Serialize JSON to file
  ConfigJson.printTo(ConfigFile);
  ConfigFile.close();

  return true;
}

// ----------------------------------------------------------------------------
// void ProcessSendcmd(const String& topicStr, const String& message)
//   Callback function for Subscribe to "Aquarium/SendCmd"
// ----------------------------------------------------------------------------
void ProcessSendcmd(const String& topicStr, const String& message) {
  // Serial.println("message received from " + topicStr + ": " + message);
  // client.publish(MqttDebugTopic, ".");
  if (message.equals("PH4")) {
    client.publish(MqttDebugTopic, "Change gState to Ph4CalibrateMode");
    gState = Ph4CalibrateMode;
  } else if (message.equals("PH7")) {
    client.publish(MqttDebugTopic, "Change gState to Ph7CalibrateMode");
    gState = Ph7CalibrateMode;
  } else if (message.equals("PH_Calculate")) {
    client.publish(MqttDebugTopic, "Change gState to PhCalculateMode");
    gState = PhCalculateMode;
  } else if (message.equals("EnterPhCalibrateMode")) {
    client.publish(MqttDebugTopic, "Change gState to EnterPhCalibrateMode");
    gState = EnterPhCalibrateMode;
  } else if (message.equals("EnterTdsCalibrateMode")) {
    client.publish(MqttDebugTopic, "Change gState to EnterTdsCalibrateMode");
    gState = EnterTdsCalibrateMode;
  } else if (message.equals("RESET")) {
    client.publish(MqttDebugTopic, "Reset ESP32 device");
    delay(1000);
    resetFunc();  // call reset
  } else if (message.equals("RequestLightSetting")) {
    String MsgStr = "";
    MsgStr = MsgStr + "{\"LightOnHour\":" + gLightOnHour + ", " 
                    +  "\"LightOnMin\":" + gLightOnMin + ", " 
                    +  "\"LightOffHour\":" + gLightOffHour + ", " 
                    +  "\"LightOffMin\":" + gLightOffMin + ", " 
                    +  "\"LightRelay\":" + gLightRelay + "}";
    client.publish("Aquarium/SendLightSetting", MsgStr);
  } else if (message.equals("RequestCo2Setting")) {
    String MsgStr = "";
    MsgStr = MsgStr + "{\"Co2OnHour\":" + gCo2OnHour + ", " 
                    +  "\"Co2OnMin\":" + gCo2OnMin + ", " 
                    +  "\"Co2OffHour\":" + gCo2OffHour + ", " 
                    +  "\"Co2OffMin\":" + gCo2OffMin + ", " 
                    +  "\"Co2Relay\":" + gCo2Relay + "}";
    client.publish("Aquarium/SendCo2Setting", MsgStr);
  } else if (message.equals("RequestPhLimitSetting")) {
    String MsgStr = "";
    MsgStr = MsgStr + "{\"PhLowLimit\":" + gPhLowLimit + ", " 
                    +  "\"PhHighLimit\":" + gPhHighLimit + "}";
    client.publish("Aquarium/SendPhLimitSetting", MsgStr);
  } else {

    
    client.publish(MqttDebugTopic, message);
  }
}
// ----------------------------------------------------------------------------
// void ProcessSetLightOnCmd(const String& topicStr, const String& message)
//  3Callback function for Subscribe to "Aquarium/SendCmd"
// ----------------------------------------------------------------------------
void ProcessSetLightTimeCmd(const String& topicStr, const String& message) {
  int value = 0;
  String Msg = "";
  // Serial.println("message received from " + topicStr + ": " + message);
  if (message.equals("NULL")) {
    return;
  }

  const int BufferSize = JSON_OBJECT_SIZE(4)+70;
  StaticJsonBuffer<BufferSize> JSONbuffer;  // Declaring static JSON buffer
  JsonObject& JSONdecoder = JSONbuffer.parseObject(message);
  JSONdecoder.prettyPrintTo(Serial);
  Serial.println(" ");

  if (JSONdecoder.containsKey("LightOnHour")) {
    value = JSONdecoder["LightOnHour"];
    Serial.println("Set LightOnHour: " + String(value));
    gLightOnHour = JSONdecoder["LightOnHour"];
  }
  if (JSONdecoder.containsKey("LightOnMin")) {
    value = JSONdecoder["LightOnMin"];
    Serial.println("Set LightOnMin: " + String(value));
    gLightOnMin = JSONdecoder["LightOnMin"];
  }
  if (JSONdecoder.containsKey("LightOffHour")) {
    value = JSONdecoder["LightOffHour"];
    Serial.println("Set LightOffHour: " + String(value));
    gLightOffHour = JSONdecoder["LightOffHour"];
  }
  if (JSONdecoder.containsKey("LightOffMin")) {
    value = JSONdecoder["LightOffMin"];
    Serial.println("Set LightOffMin: " + String(value));
    gLightOffMin = JSONdecoder["LightOffMin"];
  }
  Msg = "Light ON  - " + String(gLightOnHour) + ":" + String(gLightOnMin);
  client.publish(MqttDebugTopic, Msg);
  Msg = "Light OFF - " + String(gLightOffHour) + ":" + String(gLightOffMin);
  client.publish(MqttDebugTopic, Msg);
  SaveConfig();
}
// ----------------------------------------------------------------------------
// void ProcessContorlLightCmd(const String& topicStr, const String& message)
//  3Callback function for Subscribe to "Aquarium/SendCmd"
// ----------------------------------------------------------------------------
void ProcessControlLightCmd(const String& topicStr, const String& message) {
  // Serial.println("message received from " + topicStr + ": " + message);
  if (message.equals("true")) {
    gLightForceControl = FORCE_ON;
  } else if (message.equals("false")) {
    gLightForceControl = FORCE_OFF;
  } else if (message.equals("auto")) {
    gLightForceControl = AUTO;
  }
}
// ----------------------------------------------------------------------------
// void ProcessSetCo2OnCmd(const String& topicStr, const String& message)
//  3Callback function for Subscribe to "Aquarium/SendCmd"
// ----------------------------------------------------------------------------
void ProcessSetCo2TimeCmd(const String& topicStr, const String& message) {
  int value = 0;
  String Msg = "";
  // Serial.println("message received from " + topicStr + ": " + message);
  if (message.equals("NULL")) {
    return;
  }

  const int BufferSize = JSON_OBJECT_SIZE(4)+60;
  StaticJsonBuffer<BufferSize> JSONbuffer;  // Declaring static JSON buffer
  JsonObject& JSONdecoder = JSONbuffer.parseObject(message);
  JSONdecoder.prettyPrintTo(Serial);
  Serial.println(" ");

  if (JSONdecoder.containsKey("Co2OnHour")) {
    value = JSONdecoder["Co2OnHour"];
    Serial.println("Set Co2OnHour: " + String(value));
    gCo2OnHour = JSONdecoder["Co2OnHour"];
  }
  if (JSONdecoder.containsKey("Co2OnMin")) {
    value = JSONdecoder["Co2OnMin"];
    Serial.println("Set Co2OnMin: " + String(value));
    gCo2OnMin = JSONdecoder["Co2OnMin"];
  }
  if (JSONdecoder.containsKey("Co2OffHour")) {
    value = JSONdecoder["Co2OffHour"];
    Serial.println("Set Co2OffHour: " + String(value));
    gCo2OffHour = JSONdecoder["Co2OffHour"];
  }
  if (JSONdecoder.containsKey("Co2OffMin")) {
    value = JSONdecoder["Co2OffMin"];
    Serial.println("Set Co2OffMin: " + String(value));
    gCo2OffMin = JSONdecoder["Co2OffMin"];
  }
  Msg = "Co2 ON  - " + String(gCo2OnHour) + ":" + String(gCo2OnMin);
  client.publish(MqttDebugTopic, Msg);
  Msg = "Co2 OFF - " + String(gCo2OffHour) + ":" + String(gCo2OffMin);
  client.publish(MqttDebugTopic, Msg);
  SaveConfig();
}
// ----------------------------------------------------------------------------
// void ProcessSetPhLimitCmd(const String& topicStr, const String& message)
//  3Callback function for Subscribe to "Aquarium/SendCmd"
// ----------------------------------------------------------------------------
void ProcessSetPhLimitCmd(const String& topicStr, const String& message) {
  int value = 0;
  String Msg = "";
  // Serial.println("message received from " + topicStr + ": " + message);
  if (message.equals("NULL")) {
    return;
  }

  const int BufferSize = JSON_OBJECT_SIZE(2)+40;
  StaticJsonBuffer<BufferSize> JSONbuffer;  // Declaring static JSON buffer
  JsonObject& JSONdecoder = JSONbuffer.parseObject(message);
  JSONdecoder.prettyPrintTo(Serial);
  Serial.println(" ");

  if (JSONdecoder.containsKey("PhLowLimit")) {
    value = JSONdecoder["PhLowLimit"];
    Serial.println("Set PhLowLimit: " + String(value));
    gPhLowLimit = float(JSONdecoder["PhLowLimit"]);
  }
  if (JSONdecoder.containsKey("PhHighLimit")) {
    value = JSONdecoder["PhHighLimit"];
    Serial.println("Set PhHighLimit: " + String(value));
    gPhHighLimit = float(JSONdecoder["PhHighLimit"]);
  }
  Msg = "PH Limit - " + String(gPhLowLimit) + "~" + String(gPhHighLimit);
  client.publish(MqttDebugTopic, Msg);
  SaveConfig();
}

// ----------------------------------------------------------------------------
// void ProcessContorlCo2Cmd(const String& topicStr, const String& message)
//  3Callback function for Subscribe to "Aquarium/SendCmd"
// ----------------------------------------------------------------------------
void ProcessControlCo2Cmd(const String& topicStr, const String& message) {
  // Serial.println("message received from " + topicStr + ": " + message);
  if (message.equals("true")) {
    gCo2ForceControl = FORCE_ON;
  } else if (message.equals("false")) {
    gCo2ForceControl = FORCE_OFF;
  } else if (message.equals("auto")) {
    gCo2ForceControl = AUTO;
  }
}

// ----------------------------------------------------------------------------
// void NormalLoop()
//   Normal run loop
// ----------------------------------------------------------------------------
void NormalLoop() {
  unsigned long CurrentMillis = millis();
  static unsigned long Loop5secPreviousMillis = 0;
  static unsigned long Loop1minPreviousMillis = 0;
  static float PhVoltage, mTdsVoltage;
  int16_t adc0, adc1;
  float volts0, volts1, mEcValue, mEcValue25;

  if (CurrentMillis - Loop5secPreviousMillis >= 5000) {
    Loop5secPreviousMillis = CurrentMillis;
    String Str = gtimeClient.getFormattedTime() + " --- Loop_5_Sec()";
    // client.publish(MqttDebugTopic, Str);
    gtimeClient.update();
    LightRelayControl();
    Co2RelayControl();

    // Read PH
    adc0 = gAds1115.readADC_SingleEnded(0);
    volts0 = gAds1115.computeVolts(adc0);
    gPhArray[gPhArrayIndex] = volts0;
    Str = "PhArray[" + String(gPhArrayIndex) + "] = "+ volts0;
    client.publish("Aquarium/DebugPh", Str);
    // gPhArray[gPhArrayIndex] = random(0, 100);
    gPhArrayIndex++;
    if (gPhArrayIndex == PH_ARRAY_LENGTH) {
      gPhArrayIndex = 0;
    }

    // Read TDS
    adc1 = gAds1115.readADC_SingleEnded(1);
    volts1 = gAds1115.computeVolts(adc1);
    gTdsArray[gTdsArrayIndex] = volts1;
    Str = "TdsArray[" + String(gTdsArrayIndex) + "] = "+ volts1;
    client.publish("Aquarium/DebugTds", Str);
    // gTdsArray[gTdsArrayIndex] = random(0, 100);
    gTdsArrayIndex++;
    if (gTdsArrayIndex == TDS_ARRAY_LENGTH) {
      gTdsArrayIndex = 0;
    }

    // Read Temperature
    TemperatureInWater.requestTemperatures();
    gTemperatureArray[gTemperatureArrayIndex] = TemperatureInWater.getTempCByIndex(0);
    gTemperatureArrayIndex++;
    if (gTemperatureArrayIndex == TEMPERATURE_ARRAY_LENGTH) {
      gTemperatureArrayIndex = 0;
    }
    Serial.println ("[PH][A0] ADC: "+ String(adc1) + ", Voltage: " + String(volts1) + " V.   [TDS][A1] ADC: "+ String(adc0) + ", Voltage: " + String(volts0)+ " V");
  
  }

  if (CurrentMillis - Loop1minPreviousMillis >= 60000) {
    Loop1minPreviousMillis = CurrentMillis;
    String Str = gtimeClient.getFormattedTime() + " --- Loop_1_min()";
    // client.publish(MqttDebugTopic, Str);

    // Publish PH Value
    PhVoltage = AvergeSensorSamplingArray(gPhArray, PH_ARRAY_LENGTH, "Aquanrium/DebugPh");
    gPhValue = gPhSlope * PhVoltage + gPhOffset;
    //gPhValue = (random(62, 70))/10.0;
    
    // Publish Temperature Value
    gTemperatureValue = AvergeSensorSamplingArray(gTemperatureArray, TEMPERATURE_ARRAY_LENGTH, "Aquanrium/DebugTds");

    // Publish TDS Value
    mTdsVoltage = AvergeSensorSamplingArray(gTdsArray, TDS_ARRAY_LENGTH, "Aquanrium/DebugTemp");
    mEcValue = (133.42 * mTdsVoltage * mTdsVoltage * mTdsVoltage - 255.86 * mTdsVoltage * mTdsVoltage + 857.39 * mTdsVoltage) * gTdsKValue;
    mEcValue25 = mEcValue / (1.0 + 0.02 * (gTemperatureValue - 25.0));  // temperature compensation
    gTdsValue = mEcValue25 * gTdsFactor;

    // Publish
    String MsgStr = "";
    MsgStr = MsgStr + "{\"temp\":" + gTemperatureValue + ", " + "\"ph\":" + gPhValue + ", " + "\"tds\":" + gTdsValue + ", " + "\"light\":" + gLightRelay + ", " + "\"co2\":" + gCo2Relay + "}";
    client.publish("Aquarium/SendData", MsgStr);
  }
}

// ----------------------------------------------------------------------------
// void PhCalibrateLoop()
//   PH calibrate loop
// ----------------------------------------------------------------------------
void PhCalibrateLoop() {
  unsigned long CurrentMillis = millis();
  static unsigned long SamplingTime = millis();

  float mPhVoltageRead;
  String mMessage = "";

  if (CurrentMillis - SamplingTime > 1000) {
    SamplingTime = CurrentMillis;
    // (ADC)
    // ====== use ESP32 or EPS8266 ======
    // https://randomnerdtutorials.com/esp32-adc-analog-read-arduino-ide/
    // ESP32S(ESP WROOM-32)     Analog reading region (12 bit) is 0~4095 that is mapping to 0~3.3V. So, read voltage formula is analogRead(PIN)/4095*3.3;
    // ESP8266 Nodemcu(ESP8266) Analog reading region (10 bit) is 0~1023 that is mapping to 0~3.3V. So, read voltage formula is analogRead(PIN)/1024*3.3;
    // Arduino(ATMEGA328P)      Analog reading region (10 bit) is 0~1023 that is mapping to 0~5.0V. So, read voltage formula is analogRead(PIN)/1024*5.0
    // ====== use ADS1115 ======
    // [ADC0-PH7] ADC: 8212, Voltage: 1.54 V
    // [ADC0-PH4] ADC: 10994, Voltage: 2.06 V
    int16_t adc0;
    float volts0;
    adc0 = gAds1115.readADC_SingleEnded(0);
    volts0 = gAds1115.computeVolts(adc0);
    mPhVoltageRead = volts0;

    // Publish the message
    if (gState == Ph4CalibrateMode) {
      mMessage = "[" + gtimeClient.getFormattedTime() + "] PH4 Sampling..Index: " + gPhArrayIndex + " ,Voltage: " + mPhVoltageRead;
      // mMessage = "[" + gtimeClient.getFormattedTime() + "] PH4 Sampling..Index: " + gPhArrayIndex + " ,ADC: " + String(adc0) +", Voltage: " + String(volts0);
    } else if (gState == Ph7CalibrateMode) {
      mMessage = "[" + gtimeClient.getFormattedTime() + "] PH7 Sampling..Index: " + gPhArrayIndex + " ,Voltage: " + mPhVoltageRead;
    }
    client.publish(MqttDebugTopic, mMessage);

    // Put the Voltage into array
    gPhArray[gPhArrayIndex] = mPhVoltageRead;
    gPhArrayIndex++;

    // Calcute the average of array vale
    if (gPhArrayIndex == PH_ARRAY_LENGTH) {
      if (gState == Ph4CalibrateMode) {
        gPh4Voltage = AvergeSensorSamplingArray(gPhArray, PH_ARRAY_LENGTH, "NULL");
        mMessage = "gPh4Voltage: ";
        mMessage.concat(gPh4Voltage);
        client.publish(MqttDebugTopic, mMessage);
        client.publish(MqttDebugTopic, "PH4 Calibrating Done..");
      } else if (gState == Ph7CalibrateMode) {
        gPh7Voltage = AvergeSensorSamplingArray(gPhArray, PH_ARRAY_LENGTH, "NULL");
        mMessage = "gPh7Voltage: ";
        mMessage.concat(gPh7Voltage);
        client.publish(MqttDebugTopic, mMessage);
        client.publish(MqttDebugTopic, "PH7 Calibrating Done..");
      }
      client.publish(MqttDebugTopic, "..");
      client.publish(MqttDebugTopic, "Start Calculating....");

      gPhArrayIndex = 0;
      gState = PhCalculateMode;
    }
  }
}

// ----------------------------------------------------------------------------
// void PhCalculate()
//   PH Calculate loop
// ----------------------------------------------------------------------------
void PhCalculate() {
  // https://www.epa.gov.tw/DisplayFile.aspx?FileID=693C1D9534289901S
  // voltage7, gPh4Voltage
  // V7*S + O = 7
  // V4*S + O = 4
  // S = (7-4) / (V7-V4)
  // O =  7 - V7*S
  gPhSlope = (7 - 4) / (gPh7Voltage - gPh4Voltage);
  gPhOffset = 7 - gPhSlope * gPh7Voltage;

  client.publish(MqttDebugTopic, "gPh4Voltage: " + String(gPh4Voltage));
  client.publish(MqttDebugTopic, "gPh7Voltage: " + String(gPh7Voltage));
  client.publish(MqttDebugTopic, "gPhSlope: " + String(gPhSlope));
  client.publish(MqttDebugTopic, "gPhOffset: " + String(gPhOffset));
  client.publish(MqttDebugTopic, "PH Calibrating Done..");
  SaveConfig();
  client.publish(MqttDebugTopic, "Saving Calibration Data!");
  gState = NormalMode;
}

// ----------------------------------------------------------------------------
// void ProcessTdsCalibrateCmd(const String& topicStr, const String& message)
//  Callback function for Subscribe to "Aquarium/SendCmd"
// ----------------------------------------------------------------------------
void ProcessTdsCalibrateCmd(const String& topicStr, const String& message) {
  // Serial.println("message received from " + topicStr + ": " + message);
  if (message.equals("NULL")) {
    client.publish(MqttDebugTopic, "Please enter ppm TEXT box...");
    return;
  }
  gState = TdsCalibrateMode;
  gCalibrateTdsTarget = message.toInt();
  client.publish(MqttDebugTopic, "Start TDS Calibrate with " + message + " ppm...");
}
// ----------------------------------------------------------------------------
// void TdsCalibrateLoop()
//   TDS calibrate loop
// ----------------------------------------------------------------------------
void TdsCalibrateLoop() {
  unsigned long CurrentMillis = millis();
  static unsigned long SamplingTime = millis();

  float mTdsVoltageRead;
  String mMessage = "";

  if (CurrentMillis - SamplingTime > 1000) {
    SamplingTime = CurrentMillis;
    int16_t adc1;
    float volts1;
    adc1 = gAds1115.readADC_SingleEnded(1);
    volts1 = gAds1115.computeVolts(adc1);
    mTdsVoltageRead = adc1;
    // mTdsVoltageRead = random(0, 100);

    // Publish the message
    mMessage = "[" + gtimeClient.getFormattedTime() + "] TDS Sampling..Index: " + gTdsArrayIndex + " ,Voltage: " + mTdsVoltageRead;
    client.publish(MqttDebugTopic, mMessage);

    // Put the Voltage into array
    gTdsArray[gTdsArrayIndex] = mTdsVoltageRead;
    gTdsArrayIndex++;

    // Calcute the average of array vale
    if (gTdsArrayIndex == TDS_ARRAY_LENGTH) {
      gTdsVoltage = AvergeSensorSamplingArray(gTdsArray, TDS_ARRAY_LENGTH, "NULL");
      mMessage = "gTdsVoltage: ";
      mMessage.concat(gTdsVoltage);
      client.publish(MqttDebugTopic, mMessage);
      client.publish(MqttDebugTopic, "TDS Sampling Done..");
      client.publish(MqttDebugTopic, "  ");
      client.publish(MqttDebugTopic, "Start Calculating....");

      gTdsArrayIndex = 0;
      gState = TdsCalculateMode;
    }
  }
}
// ----------------------------------------------------------------------------
// void TdsCalculate()
//   PH Calculate loop
// ----------------------------------------------------------------------------
void TdsCalculate() {
  // http://www.prowatertech.com.tw/index_inner_info_skill_detail.asp?id=57
  // http://140.112.183.23/YunLin/EC2CF2PPM.pdf
  // 1 ms/cm=1000 uS/cm
  // 1ppm(TDS)=1mg/L=2uS/cm
  // -----------------------------
  // | EC    | 美 TDS  | 歐 TDS  |
  // -----------------------------
  // | mS/cm | 0.5ppm  | 0.64ppm |
  // -----------------------------
  float mTargetEc;

  mTargetEc = gCalibrateTdsTarget / gTdsFactor;
  mTargetEc = mTargetEc * (1.0 + 0.02 * (gTemperatureValue - 25.0));

  gTdsKValue = mTargetEc / (133.42 * gTdsVoltage * gTdsVoltage * gTdsVoltage - 255.86 * gTdsVoltage * gTdsVoltage + 857.39 * gTdsVoltage);
  client.publish(MqttDebugTopic, "gTdsKValue: " + String(gTdsKValue));
  client.publish(MqttDebugTopic, "TDS Calibrating Done..");
  SaveConfig();
  client.publish(MqttDebugTopic, "Saving Calibration Data!");
  gState = NormalMode;
}

// ----------------------------------------------------------------------------
// void AvergeSensorSamplingArray()
//   Output the averge of sensor array
// ----------------------------------------------------------------------------
float AvergeSensorSamplingArray(float* Array, int ArrayNumber, String PublishTopic) {
  int Index;
  float Avg;
  float Amount = 0;
  String mMessage = "";

  // mMessage = "Before Sort: ";
  // Index = 0;
  // while (Index < ArrayNumber) {
  //   if (Index < ArrayNumber-1) {
  //     mMessage = mMessage + Array[Index] + ", ";
  //   } else {
  //     mMessage = mMessage + Array[Index];
  //   }
  //   Index++;
  // }
  // client.publish(MqttDebugTopic, mMessage);

  sortArray(Array, ArrayNumber);

    Index = 0;
    while (Index < ArrayNumber) {
      if (Index < ArrayNumber-1) {
        mMessage = mMessage + Array[Index] + ", ";
      } else {
        mMessage = mMessage + Array[Index];
      }
      Index++;
    }
    if (PublishTopic != "NULL") {
      client.publish(PublishTopic, mMessage);
    }

  for (Index = 1; Index < ArrayNumber - 1; Index++) {
    Amount += Array[Index];
  }
  Avg = (float)Amount / (ArrayNumber - 2);
  return Avg;
}

// ----------------------------------------------------------------------------
// void Co2RelayControl()
//   Co2 Relay Control
// ----------------------------------------------------------------------------
void Co2RelayControl() {
  bool mCo2yStatus = false;
  // int mCo2ButtonState = 0;
  int CurrentTimeSecs = CAL_TIME_SECS(gtimeClient.getHours(), gtimeClient.getMinutes(), gtimeClient.getSeconds());
  int gCo2OnTimeSecs = CAL_TIME_SECS(gCo2OnHour, gCo2OnMin, 0);
  int gCo2OffTimeSecs = CAL_TIME_SECS(gCo2OffHour, gCo2OffMin, 0);

  if ((gCo2OnTimeSecs < CurrentTimeSecs) and (CurrentTimeSecs < gCo2OffTimeSecs)) {
    mCo2yStatus = true;
  } else {
    mCo2yStatus = false;
  }

  // Read button status
  // mCo2ButtonState = digitalRead(CO2_SWITCH_PIN);

  // if (mCo2ButtonState == LOW) {  // Button is pressed
  //   client.publish(MqttDebugTopic, "CO2 Pass Relay...");
  //   digitalWrite(CO2_RELAY_PIN, HIGH);
  //   gCo2Relay = true;
  // } else 
  if (gCo2ForceControl != AUTO) {
    gCo2Relay = (gCo2ForceControl == FORCE_ON) ? true : false;
    if (gCo2Relay == true) {
      client.publish(MqttDebugTopic, "CO2 Web force On");
      digitalWrite(CO2_RELAY_PIN, LOW);
    } else {
      client.publish(MqttDebugTopic, "CO2 Web force Off");
      digitalWrite(CO2_RELAY_PIN, HIGH);
    }
  } else {  // Button is not pressed
    if (mCo2yStatus == true){
      if (gPhValue < gPhLowLimit) {
        mCo2yStatus = false;
      } else if (gPhValue > gPhHighLimit){
        mCo2yStatus = true;
      } else {
        mCo2yStatus = gCo2Relay;
      }
    }

    if (gCo2Relay != mCo2yStatus) {
      gCo2Relay = mCo2yStatus;
      if (gCo2Relay == true) {
        client.publish(MqttDebugTopic, "CO2 On");
        digitalWrite(CO2_RELAY_PIN, LOW);
      } else {
        client.publish(MqttDebugTopic, "CO2 Off");
        digitalWrite(CO2_RELAY_PIN, HIGH);
      }
    }
  }
}

// ----------------------------------------------------------------------------
// void LightRelayControl()
//   Light Relay Control
// ----------------------------------------------------------------------------
void LightRelayControl() {
  bool mLightyStatus = false;
  // int mLightButtonState = 0;
  int mCurrentTimeSecs = CAL_TIME_SECS(gtimeClient.getHours(), gtimeClient.getMinutes(), gtimeClient.getSeconds());
  int mLightOnTimeSecs = CAL_TIME_SECS(gLightOnHour, gLightOnMin, 0);
  int mLightOffTimeSecs = CAL_TIME_SECS(gLightOffHour, gLightOffMin, 0);

  if ((mLightOnTimeSecs < mCurrentTimeSecs) and (mCurrentTimeSecs < mLightOffTimeSecs)) {
    mLightyStatus = true;
  } else {
    mLightyStatus = false;
  }

//   // Read button status
//   mLightButtonState = digitalRead(LIGHT_SWITCH_PIN);
// 
//   if (mLightButtonState == LOW) {  // Button is pressed
//     client.publish(MqttDebugTopic, "Light Pass Relay...");
//     digitalWrite(LIGHT_RELAY_PIN, HIGH);
//     gLightRelay = true;
//   } else 
  if (gLightForceControl != AUTO) {
    gLightRelay = (gLightForceControl == FORCE_ON) ? true : false;
    if (gLightRelay == true) {
      client.publish(MqttDebugTopic, "Light Web force On");
      digitalWrite(LIGHT_RELAY_PIN, LOW);
    } else {
      client.publish(MqttDebugTopic, "Light Web force Off");
      digitalWrite(LIGHT_RELAY_PIN, HIGH);
    }
  } else {  // Button is not pressed
    if (gLightRelay != mLightyStatus) {
      gLightRelay = mLightyStatus;
      if (gLightRelay == true) {
        client.publish(MqttDebugTopic, "Light On");
        digitalWrite(LIGHT_RELAY_PIN, LOW);
      } else {
        client.publish(MqttDebugTopic, "Light Off");
        digitalWrite(LIGHT_RELAY_PIN, HIGH);
      }
    }
  }
}
