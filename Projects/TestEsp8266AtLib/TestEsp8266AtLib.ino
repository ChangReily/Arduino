#include <SoftwareSerial.h>
#include <ESP8266AT.h>

#include <TimeLib.h>
#include <TimeAlarms.h>

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <OneWire.h>
#include <DallasTemperature.h>

#define DEBUG_8266 0
#define SNTP_YEAR(Str) Str.substring(19,23)
#define SNTP_MONTH(Str) Str.substring(4,6)
#define SNTP_DAY(Str) Str.substring(7,9)
#define SNTP_HOUR(Str) Str.substring(10,12)
#define SNTP_MIN(Str) Str.substring(13,15)
#define SNTP_SEC(Str) Str.substring(16,19)

SoftwareSerial EspSerial(10, 11);
ESP8266AT Esp01(EspSerial);

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.

#define PIR_IN_PIN 3
#define LIGHT_RELAY_OUT_PIN 4
#define FEEDER_OUT_PIN 9

int LightStatus = 0;

byte termometru[8] = //icon for termometer
{
  B00100,
  B01010,
  B01010,
  B01110,
  B01110,
  B11111,
  B11111,
  B01110
};

byte picatura[8] = //icon for water droplet
{
  B01110,
  B10001,
  B10101,
  B10101,
  B01110,
  B01110,
  B00100,
  B00000,
};

void setup(void)
{
  //--------------------------------------------------------------------------------
  // Initialize Serial Port for debug
  Serial.begin(115200);

  //--------------------------------------------------------------------------------
  // Initialize LCD1602 I2C display model
  lcd.init();
  lcd.backlight();
  lcd.createChar(1, termometru);
  lcd.createChar(2, picatura);

  //--------------------------------------------------------------------------------
  // Initialize DS1802 Temperature model
  sensors.begin();

  //--------------------------------------------------------------------------------
  // Initialize I/O PIN for HC-SR501 PIR Sensor
  pinMode(PIR_IN_PIN, INPUT);

  //--------------------------------------------------------------------------------
  // Initialize I/O PIN for Light Relay
  pinMode(LIGHT_RELAY_OUT_PIN, OUTPUT);

  //--------------------------------------------------------------------------------
  // Initialize I/O PIN for Fish Feeder
  pinMode(FEEDER_OUT_PIN, OUTPUT);

  //--------------------------------------------------------------------------------
  // Initialize ESP8266 ESP-01 for AT command check, set SNTP config, and get AP

  while (!Esp01.ExecAT()) {
    delay(2000);
  }

  while (Esp01.SetATCIPSNTPCFG() == "None") {
    delay(2000);
  }
  delay(2000);

  int Counter=0;
  while (Counter < 20) {
    lcd.setCursor(0, 0);
    lcd.print("Connecting...");
    lcd.setCursor(0, 1);
    lcd.print(Esp01.QueryATCWJAPCUR());
    if (Esp01.QueryATCWJAPCUR() != "No AP"){
       delay(2000);
      lcd.clear();
      break;  
    }
    delay(3000);
    Counter++;
  }
  
  //--------------------------------------------------------------------------------
  // Get the SNTP time and set the time
  SynSntpTime();

  //--------------------------------------------------------------------------------
  // Display the time on LCD1602
  lcd.setCursor(0, 0);
  lcd.print(String(year()) + "-" + ByteString(month()) + "-" + ByteString(day()) + " " + ByteString(hour()) + ":" + ByteString(minute()));

  // Display Temperatures
  lcd.setCursor(0, 1);
  lcd.write(0x01);
  lcd.print(" 00.00");
  lcd.write(0xDF);
  lcd.print("C");

  // Display LightStatus
  lcd.print("  ");
  lcd.write(0x02);
  if (LightStatus == 1) {
    lcd.print(" ON ");
  } else {
    lcd.print(" OFF");
  }

  //--------------------------------------------------------------------------------
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
  Alarm.timerRepeat(60, Repeat60sec);      // timer for every 60 seconds

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
//  Serial.println(SntpTime);
  if (SntpTime.substring(19, 23).toInt() != 1970) {
    setTime(SNTP_HOUR(SntpTime).toInt(), SNTP_MIN(SntpTime).toInt(), SNTP_SEC(SntpTime).toInt(), SNTP_DAY(SntpTime).toInt(), SNTP_MONTH(SntpTime).toInt(), SNTP_YEAR(SntpTime).toInt());
    if (makeTime(8, 30, 0, SNTP_DAY(SntpTime).toInt(), SNTP_MONTH(SntpTime).toInt(), SNTP_YEAR(SntpTime).toInt()) < now() &&
        now() < makeTime(20, 30, 0, SNTP_DAY(SntpTime).toInt(), SNTP_MONTH(SntpTime).toInt(), SNTP_YEAR(SntpTime).toInt())) {
      LightOn();
    } else {
      LightOff();
    }
  }
}

void LightOn() {
  if (LightStatus==0){
    digitalClockDisplay();
    Serial.println("  [Light] - Turn Lights On");
    digitalWrite(LIGHT_RELAY_OUT_PIN, HIGH);
    LightStatus=1;
  }
}

void LightOff() {
  if (LightStatus==1){
    digitalClockDisplay();
    Serial.println("  [Light] - Turn Lights Off");
    digitalWrite(LIGHT_RELAY_OUT_PIN, LOW);
    LightStatus=0;
  }
}

void FeederOn() {
  digitalClockDisplay();
  Serial.println("  [Feeder] Fish Feeder Start");
  digitalWrite(FEEDER_OUT_PIN, HIGH);
}

void FeederOff() {
  digitalClockDisplay();
  Serial.println("  [Feeder] Fish Feeder End");
  digitalWrite(FEEDER_OUT_PIN, LOW);
}

void Repeat60sec() {
//  digitalClockDisplay();
//  Serial.println("  60 second timer");
  
  if (year() == 1970) {
   SynSntpTime();
  }
  // Update Time on LCD1602
  lcd.setCursor(0, 0);
  lcd.print(String(year()) + "-" + ByteString(month()) + "-" + ByteString(day()) + " " + ByteString(hour()) + ":" + ByteString(minute()));

  // Update Temperatures on LCD1602
  sensors.requestTemperatures();
  lcd.setCursor(0, 1);
  lcd.write(0x01);
  lcd.print(" ");
  lcd.print(sensors.getTempCByIndex(0));// 取得溫度讀數（攝氏）並輸出， 參數0代表匯流排上第0個1-Wire裝置
  lcd.write(0xDF);
  lcd.print("C");

  // Update Light Status
  lcd.print("  ");
  lcd.write(0x02);
  if (LightStatus == 1) {
    lcd.print(" ON ");
  } else {
    lcd.print(" OFF");
  }
}

void Repeats3sec() {
//  digitalClockDisplay();
//  Serial.println("  3 second timer");

  int val = digitalRead(PIR_IN_PIN); //讀取 PIR 輸出
  if (val == HIGH) { //PIR 有偵測到時
    digitalClockDisplay();
    Serial.println("  3 second timer [PIR = 1] LCD ON");
    lcd.backlight();
  } else {
//    digitalClockDisplay();
//    Serial.println("  3 second timer [PIR = 0] LCD OFF");
    lcd.noBacklight();
  }
}


