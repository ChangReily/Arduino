#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266AT.h>

#define DEBUG 0
//--------------------------------------------------------------------------------
// Define IO PIN
#define PIR_IN_PIN 3
#define LIGHT_RELAY_OUT_PIN 4
#define FEEDER_OUT_PIN 9
#define TEMPERATURE_IO_PIN 2

//--------------------------------------------------------------------------------
// Declare the alarm structrun
#define AlarmHMS(_hr_, _min_, _sec_) (_hr_ * SECS_PER_HOUR + _min_ * SECS_PER_MIN + _sec_)

enum Alarm_Type {
  NotAllocated,
  Timer,
  Daily
};

typedef void (*OnTick_t)();

struct Alarm_Info {
  OnTick_t TriggerHandle;
  time_t value = 0;
  time_t NextTrigger = 0;
  Alarm_Type AlarmType = NotAllocated;
};

//--------------------------------------------------------------------------------
// Declare the LCD char
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

//--------------------------------------------------------------------------------
// Declare global variable
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display
SoftwareSerial EspSerial(10, 11);
ESP8266AT Esp01(EspSerial);
OneWire oneWire(TEMPERATURE_IO_PIN);
DallasTemperature sensors(&oneWire); // Pass our oneWire reference to Dallas Temperature.
Alarm_Info AlarmList[10];
int LightStatus = 0;

//--------------------------------------------------------------------------------
// Setup function
//--------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) ; // wait for serial
  delay(200);

  //--------------------------------------------------------------------------------
  // Initialize Timer
  setSyncProvider(RTC.get);
  setSyncInterval(1800);

  //--------------------------------------------------------------------------------
  // Initialize LCD1602 I2C display model
  lcd.init();
  lcd.clear();
  lcd.noBacklight();
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

  int Counter = 0;
  while (Counter < 20) {
    lcd.setCursor(0, 0);
    lcd.print("Connecting...");
    lcd.setCursor(0, 1);
    lcd.print(Esp01.QueryATCWJAPCUR());
    if (Esp01.QueryATCWJAPCUR() != "No AP") {
      delay(2000);
      lcd.clear();
      break;
    }
    delay(3000);
    Counter++;
  }

  //--------------------------------------------------------------------------------
  // Create Daily Alarm
  AlarmCreate(8,  30, 0, Daily, LightOn);    // 8:30 every day
  AlarmCreate(20, 30, 0, Daily, LightOff);   // 20:30 every day

  // Create Daily Alarm for Feeder
  AlarmCreate(8, 45, 0, Daily, FeederOn);
  AlarmCreate(8, 45, 3, Daily, FeederOff);
  AlarmCreate(20, 45, 0, Daily, FeederOn);
  AlarmCreate(20, 45, 3, Daily, FeederOff);

  // Set Alarm for Sync Time
  AlarmCreate(7, 30, 0, Daily, SynSntpTimeToRtc);   // 7:30 every day

  //--------------------------------------------------------------------------------
  // Create Timer Alarm
  //AlarmCreate(0, 0, 1, Timer, Repeat1sec);   // 1 seccond
  AlarmCreate(0, 0, 3, Timer, Repeat3sec);        // 3 seccond
  AlarmCreate(0, 0, 60, Timer, Repeat60sec);      // 60 seccond/1 minute

  //--------------------------------------------------------------------------------
  // Update information
  if (makeTime(8, 30, 0, day(), month(), year()) < now() && now() < makeTime(20, 30, 0, day(), month(), year())) {
    delay(2000); // Refresh tm cache
    LightOn();
  } else {
    delay(2000); // Refresh tm cache
    LightOff();
  }

  UpdateLcdInfo();
}

void loop() {
  for (int index = 0; index < 10; index++) {
    if (now() >= AlarmList[index].NextTrigger) {
      OnTick_t TickHandler = AlarmList[index].TriggerHandle;
      updateNextTrigger(index);
      if (TickHandler != NULL) {
        (*TickHandler)();
      }
    }
  }
  delay(1000);
}

void AlarmCreate(const int H, const int M, const int S, Alarm_Type CreateAlarmType, OnTick_t CreateTriggerHandler) {
  int index;
  for (index = 0; index < 10; index++) {
    if (AlarmList[index].AlarmType == NotAllocated) {
      AlarmList[index].AlarmType = CreateAlarmType;
      AlarmList[index].TriggerHandle = CreateTriggerHandler;
      AlarmList[index].value = AlarmHMS(H, M, S);
      updateNextTrigger(index);
      break;
    }
  }
  if (index == 10) {
    Serial.println("Out of range");
  }
}

time_t updateNextTrigger(int AlarmListIndex)
{
  time_t time = now();
  //  Serial.println(time);
  if ((AlarmList[AlarmListIndex].NextTrigger) <= time) {
    // update alarm if next trigger is not yet in the future
    if (AlarmList[AlarmListIndex].AlarmType == Daily) {
      //if this is a daily alarm
      //Serial.println("if this is a daily alarm");
      if (AlarmList[AlarmListIndex].value + previousMidnight(now()) <= time) {
        // if time has passed then set for tomorrow
        //Serial.println("if time has passed then set for tomorrow");
        AlarmList[AlarmListIndex].NextTrigger = AlarmList[AlarmListIndex].value + nextMidnight(time);
        //Serial.println("if time has passed then set for tomorrow");
      } else {
        // set the date to today and add the time given in value
        // Serial.println("set the date to today and add the time given in value");
        AlarmList[AlarmListIndex].NextTrigger = AlarmList[AlarmListIndex].value + previousMidnight(time);
      }
    }
  }
  if (AlarmList[AlarmListIndex].AlarmType == Timer) {
    // its a timer
    AlarmList[AlarmListIndex].NextTrigger = time + AlarmList[AlarmListIndex].value;  // add the value to previous time (this ensures delay always at least Value seconds)
  }
}

void UpdateLcdInfo() {
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

String ByteString(int Num) {
  if (Num < 10) {
    return  "0" + String(Num);
  } else {
    return  String(Num);
  }
}
void digitalClockDisplay(String Str) {
  // digital clock display of the time
#if DEBUG  
  if (Str == "") {
    Serial.println(String(year()) + "-" + ByteString(month()) + "-" + ByteString(day()) + " " + ByteString(hour()) + ":" + ByteString(minute()) + ":" + ByteString(second()));
  } else {
    Serial.println(String(year()) + "-" + ByteString(month()) + "-" + ByteString(day()) + " " + ByteString(hour()) + ":" + ByteString(minute()) + ":" + ByteString(second()) + " " + Str);
  }
#endif
}

void LightOn() {
  if (LightStatus == 0) {
    digitalClockDisplay("[Light] - Turn Lights On");
    digitalWrite(LIGHT_RELAY_OUT_PIN, HIGH);
    LightStatus = 1;
  }
}

void LightOff() {
  if (LightStatus == 1) {
    digitalClockDisplay("[Light] - Turn Lights Off");
    digitalWrite(LIGHT_RELAY_OUT_PIN, LOW);
    LightStatus = 0;
  }
}

void FeederOn() {
  digitalClockDisplay("[Feeder] Fish Feeder Start");
  digitalWrite(FEEDER_OUT_PIN, HIGH);
}

void FeederOff() {
  digitalClockDisplay("[Feeder] Fish Feeder End");
  digitalWrite(FEEDER_OUT_PIN, LOW);
}

void SynSntpTimeToRtc() {
  digitalClockDisplay("[Sync] SNTP Time");
  if (Esp01.QueryATCWJAPCUR() != "No AP") {
    String SntpTime;
    SntpTime = Esp01.GetSntpTime();
#if DEBUG
    Serial.print("SNTP : ");
    Serial.println(SntpTime);
#endif
    if (SNTP_YEAR(SntpTime).toInt() != 1970) {
      RTC.set(makeTime(SNTP_HOUR(SntpTime).toInt(), SNTP_MIN(SntpTime).toInt(), SNTP_SEC(SntpTime).toInt(), SNTP_DAY(SntpTime).toInt(), SNTP_MONTH(SntpTime).toInt(), SNTP_YEAR(SntpTime).toInt()));
    }
  }
}

void Repeat1sec() {
  digitalClockDisplay("");
  //  lcd.setCursor(0, 0);
  //  lcd.print(String(year()) + "-" + ByteString(month()) + "-" + ByteString(day()));
  //  lcd.setCursor(0, 1);
  //  lcd.print(ByteString(hour()) + ":" + ByteString(minute()) + ":" + ByteString(second()));
  //  Serial.println("-------");
  //  tmElements_t tm;
  //  RTC.read(tm);
  //  lcd.setCursor(0, 0);
  //  lcd.print(String(tmYearToCalendar(tm.Year)) + "-" + ByteString(tm.Month) + "-" + ByteString(tm.Day));
  //  lcd.setCursor(0, 1);
  //  lcd.print(ByteString(tm.Hour) + ":" + ByteString(tm.Minute) + ":" + ByteString(tm.Second));
  //  Serial.println(String(tmYearToCalendar(tm.Year)) + "-" + ByteString(tm.Month) + "-" + ByteString(tm.Day) + " " + ByteString(tm.Hour) + ":" + ByteString(tm.Minute) + ":" + ByteString(tm.Second));
}

void Repeat3sec() {
  digitalClockDisplay("3 second timer");
  int val = digitalRead(PIR_IN_PIN); //讀取 PIR 輸出
  if (val == HIGH) { //PIR 有偵測到時
    digitalClockDisplay("3 second timer [PIR = 1] LCD ON");
    lcd.backlight();
  } else {
    //    digitalClockDisplay();
    //    Serial.println("  3 second timer [PIR = 0] LCD OFF");
    lcd.noBacklight();
  }
}

void Repeat60sec() {
  digitalClockDisplay("60 second timer");
  UpdateLcdInfo();
}


