#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <LiquidCrystal_I2C.h>

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

Alarm_Info AlarmList[10];

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

#define AlarmHMS(_hr_, _min_, _sec_) (_hr_ * SECS_PER_HOUR + _min_ * SECS_PER_MIN + _sec_)

void setup() {
  Serial.begin(9600);
  while (!Serial) ; // wait for serial
  delay(200);
  
  //--------------------------------------------------------------------------------
  // Initialize Timer
  tmElements_t tm;
  RTC.read(tm);
  setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);

  //--------------------------------------------------------------------------------
  // Initialize LCD1602 I2C display model
  lcd.init();
  lcd.noBacklight();
  lcd.setCursor(0, 0);
  lcd.print(String(tmYearToCalendar(tm.Year)) + "-" + ByteString(tm.Month) + "-" + ByteString(tm.Day));
  lcd.setCursor(0, 1);
  lcd.print(ByteString(tm.Hour) + ":" + ByteString(tm.Minute) + ":" + ByteString(tm.Second));
  
  //--------------------------------------------------------------------------------
  // Create Daily Alarm
  AlarmCreate(8,  30, 0, Daily, LightOn);    // 8:30 every day
  AlarmCreate(20, 30, 0, Daily, LightOff);   // 20:30 every day
 
  // Create Daily Alarm for Feeder
  AlarmCreate(8, 45, 0, Daily,FeederOn);
  AlarmCreate(8, 45, 3, Daily,FeederOff);
  AlarmCreate(20, 45, 0, Daily,FeederOn);
  AlarmCreate(20, 45, 3, Daily,FeederOff);
  
  //--------------------------------------------------------------------------------
  // Create Timer Alarm
  AlarmCreate(0, 0, 1, Timer, XXX);   // 3 seccond
//  AlarmCreate(0, 0, 3, Timer, XXX);   // 3 seccond
//  AlarmCreate(0, 0, SECS_PER_MIN, Timer, LightOn);   // 1 minute
  
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
  for (int index = 0; index < 10; index++) {
    if (AlarmList[index].AlarmType == NotAllocated) {
      AlarmList[index].AlarmType = CreateAlarmType;
      AlarmList[index].TriggerHandle = CreateTriggerHandler;
      AlarmList[index].value = AlarmHMS(H, M, S);
      updateNextTrigger(index);
      break;
    }
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
      //      Serial.println("if this is a daily alarm");
      if (AlarmList[AlarmListIndex].value + previousMidnight(now()) <= time) {
        // if time has passed then set for tomorrow
        //        Serial.println("if time has passed then set for tomorrow");
        AlarmList[AlarmListIndex].NextTrigger = AlarmList[AlarmListIndex].value + nextMidnight(time);
        //        Serial.println("if time has passed then set for tomorrow");
      } else {
        //        Serial.println("set the date to today and add the time given in value");
        // set the date to today and add the time given in value
        AlarmList[AlarmListIndex].NextTrigger = AlarmList[AlarmListIndex].value + previousMidnight(time);
      }
    }

  }
  if (AlarmList[AlarmListIndex].AlarmType == Timer) {
    // its a timer
    AlarmList[AlarmListIndex].NextTrigger = time + AlarmList[AlarmListIndex].value;  // add the value to previous time (this ensures delay always at least Value seconds)
  }
}

void XXX() {
  Serial.println("-------");
  tmElements_t tm;
  RTC.read(tm);
  lcd.setCursor(0, 0);
  lcd.print(String(tmYearToCalendar(tm.Year)) + "-" + ByteString(tm.Month) + "-" + ByteString(tm.Day));
  lcd.setCursor(0, 1);
  lcd.print(ByteString(tm.Hour) + ":" + ByteString(tm.Minute) + ":" + ByteString(tm.Second));
  Serial.println(String(tmYearToCalendar(tm.Year)) + "-" + ByteString(tm.Month) + "-" + ByteString(tm.Day) + " " + ByteString(tm.Hour) + ":" + ByteString(tm.Minute) + ":" + ByteString(tm.Second)); 
  digitalClockDisplay();
}

void digitalClockDisplay() {
  // digital clock display of the time
 
  Serial.println(String(year()) + "-" + ByteString(month()) + "-" + ByteString(day()) + " " + ByteString(hour()) + ":" + ByteString(minute()) + ":" + ByteString(second()));
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
//  digitalWrite(FEEDER_OUT_PIN, LOW);
}

String ByteString(int Num) {
  if (Num < 10) {
    return  "0" + String(Num);
  } else {
    return  String(Num);
  }
}

