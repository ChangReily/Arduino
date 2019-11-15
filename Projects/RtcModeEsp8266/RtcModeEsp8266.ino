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


//typedef void (*OnTick_t)();  // alarm callback function typedef

Alarm_Info AlarmList[10];

LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  Serial.begin(9600);
  while (!Serial) ; // wait for serial
  delay(200);
  //--------------------------------------------------------------------------------
  // Initialize LCD1602 I2C display model
  lcd.init();
  lcd.noBacklight();
  //--------------------------------------------------------------------------------

  tmElements_t tm;
  RTC.read(tm);
  setTime(tm.Hour, tm.Minute, tm.Second, tm.Day, tm.Month, tm.Year);

  AlarmCreate(17, 47, 0, Daily, LightOn);   // 8:30 every day
  AlarmCreate(20, 30, 0, Daily, LightOn);   // 8:30 every day
   AlarmCreate(0, 0, 1, Timer, LightOn);   // 8:30 every day
}
#define AlarmHMS(_hr_, _min_, _sec_) (_hr_ * SECS_PER_HOUR + _min_ * SECS_PER_MIN + _sec_)

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
void LightOn() {
  Serial.println("ON");
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
String ByteString(int Num) {
  if (Num < 10) {
    return  "0" + String(Num);
  } else {
    return  String(Num);
  }
}
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

