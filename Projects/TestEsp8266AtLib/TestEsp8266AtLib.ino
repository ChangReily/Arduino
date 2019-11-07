#include <SoftwareSerial.h>
#include <ESP8266AT.h>

#include <TimeLib.h>
#include <TimeAlarms.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define DEBUG_8266 0
SoftwareSerial EspSerial(10, 11);
ESP8266AT Esp01(EspSerial);

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.

#define PIR_IN_PIN 2
#define LIGHT_RELAY_OUT_PIN 2


void setup(void)
{
  String SntpTime;
  //--------------------------------------------------------------------------------
  // Initialize Serial Port for debug
  Serial.begin(115200);
  //--------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------
  // Initialize LCD1602 I2C display model
  lcd.init();
  lcd.backlight();
  //--------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------
  // Initialize DS1802 Temperature model
  sensors.begin();
  //--------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------
  // Initialize I/O PIN for HC-SR501 PIR Sensor
  pinMode(PIR_IN_PIN, INPUT);
  //--------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------
  // Initialize I/O PIN for Light Relay
  pinMode(LIGHT_RELAY_OUT_PIN, OUTPUT);
  //--------------------------------------------------------------------------------

  //--------------------------------------------------------------------------------
  // Initialize ESP8266 ESP-01 for AT command check and set SNTP config
  delay(10000); // Delay 10 sec for connect internet

  while (!Esp01.ExecAT()) {
    delay(2000);
  }

  while (Esp01.SetATCIPSNTPCFG() == "None") {
    delay(2000);
  }
  //--------------------------------------------------------------------------------

#if DEBUG_8266
  Serial.print("== Checks Version Information ==\n");
  Serial.println(Esp01.ExecATGMR());

  Serial.print("\n== UART Configuration ==\n");
  Serial.println(Esp01.QueryATUARTCUR());

  Serial.print("\n== Current Wi-Fi mode ==\n");
  Serial.println(Esp01.QueryATCWMODECUR());

  Serial.print("\n== Get Current AP ==\n");
  Serial.println(Esp01.QueryATCWJAPCUR());

  Serial.print("\n== Set Configuration of SNTP ==\n");
  Serial.println(Esp01.SetATCIPSNTPCFG());

  Serial.print("\n== Get Configuration of SNTP ==\n");
  Serial.println(Esp01.QueryATCIPSNTPCFG());

  Serial.print("\n== Checks the SNTP Time ==\n");
  Serial.println(Esp01.QueryATCIPSNTPTIME());
#endif

  // Get the SNTP time first and set the time
  SntpTime = Esp01.GetSntpTime();
  setTime(SntpTime.substring(11, 13).toInt(), SntpTime.substring(14, 16).toInt(), SntpTime.substring(17, 19).toInt(), SntpTime.substring(8, 10).toInt(), SntpTime.substring(5, 7).toInt(), SntpTime.substring(0, 4).toInt());

  // Display the time on LCD1602
  lcd.setCursor(0, 0);
  SntpTime = SntpTime.substring(0, 4) + "-" + SntpTime.substring(5, 7) + "-" + SntpTime.substring(8, 10) + " " + SntpTime.substring(11, 13) + ":" + SntpTime.substring(14, 16);
  lcd.print(SntpTime);

  // Set Alarm for Light
  Alarm.alarmRepeat(8, 30, 0, LightOn);   // 8:30 every day
  Alarm.alarmRepeat(20, 30, 0, LightOff); // 20:30 every day

  // Set Alarm for Feeder
  Alarm.alarmRepeat(8, 45, 0, FeederOn);
  Alarm.alarmRepeat(8, 45, 3, FeederOff);
  Alarm.alarmRepeat(20, 45, 0, FeederOn);
  Alarm.alarmRepeat(20, 45, 3, FeederOff);

  // Set Alarm for Sync Time
  Alarm.alarmRepeat(7, 30, 0, SynSntpTime);   // 7:30 every day
  Alarm.alarmRepeat(19, 30, 0, SynSntpTime);  // 19:30 every day

  // Set Alarm for update time & Temperature on LCD1602
  Alarm.timerRepeat(60, Repeat64sec);      // timer for every 60 seconds

  // Set Alarm for HC-SR501 PIR Sensor
  Alarm.timerRepeat(3, Repeats3sec);      // timer for every 3 seconds

}

void loop(void)
{
  Alarm.delay(1000); // wait one second between clock display
}

String ByteString(int Num) {
  if (Num < 10) {
    return  "0" + String(Num);
  } else {
    return  String(Num);
  }
}

// functions to be called when an alarm triggers:
void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(String(year()) + "-" + ByteString(month()) + "-" + ByteString(day()) + " " + ByteString(hour()) + ":" + ByteString(minute()) + ":" + ByteString(second()));
}

void SynSntpTime() {
  digitalClockDisplay();
  Serial.println("  [SynTime] - Sync SNTP Time");
  String SntpTime;
  SntpTime = Esp01.GetSntpTime();
  if (SntpTime.substring(0, 4) != "1970") {
    setTime(SntpTime.substring(11, 13).toInt(), SntpTime.substring(14, 16).toInt(), SntpTime.substring(17, 19).toInt(), SntpTime.substring(8, 10).toInt(), SntpTime.substring(5, 7).toInt(), SntpTime.substring(0, 4).toInt());
  }
}
void LightOn() {
  digitalClockDisplay();
  digitalWrite(LIGHT_RELAY_OUT_PIN, LOW);
  Serial.println("  [Light] - Turn Lights On");
}

void LightOff() {
  digitalClockDisplay();
  digitalWrite(LIGHT_RELAY_OUT_PIN, HIGH);
  Serial.println("  [Light] - Turn Lights Off");
}

void FeederOn() {
  digitalClockDisplay();
  Serial.println("  [Feeder] Fish Feeder Start");
}

void FeederOff() {
  digitalClockDisplay();
  Serial.println("  [Feeder] Fish Feeder End");
}

void Repeat64sec() {
  digitalClockDisplay();
  Serial.println("  60 second timer");
  String SntpTime;
  if (year() == 1970) {
    SntpTime = Esp01.GetSntpTime();
    if (SntpTime.substring(0, 4) != "1970") {
      setTime(SntpTime.substring(11, 13).toInt(), SntpTime.substring(14, 16).toInt(), SntpTime.substring(17, 19).toInt(), SntpTime.substring(8, 10).toInt(), SntpTime.substring(5, 7).toInt(), SntpTime.substring(0, 4).toInt());
    }
  }
  // Update Time on LCD1602
  lcd.setCursor(0, 0);
  lcd.print(String(year()) + "-" + ByteString(month()) + "-" + ByteString(day()) + " " + ByteString(hour()) + ":" + ByteString(minute()));

  // Update Temperatures on LCD1602
  sensors.requestTemperatures();
  lcd.setCursor(1, 0);
  lcd.print(sensors.getTempCByIndex(0));// 取得溫度讀數（攝氏）並輸出， 參數0代表匯流排上第0個1-Wire裝置
  lcd.write(0xDF);
  lcd.print("C");
}

void Repeats3sec() {
  digitalClockDisplay();
  Serial.println("  3 second timer");

  int val = digitalRead(PIR_IN_PIN); //讀取 PIR 輸出
  if (val == HIGH) { //PIR 有偵測到時
    lcd.backlight();
  } else {
    lcd.noBacklight();
  }
}


