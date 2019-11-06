#include <SoftwareSerial.h>
#include <ESP8266AT.h>

#include <TimeLib.h>
#include <TimeAlarms.h>

SoftwareSerial EspSerial(10, 11);
ESP8266AT Esp01(EspSerial);
#define DEBUG 0

void setup(void)
{
  String SntpTime;
  Serial.begin(115200);
#if DEBUG
  Serial.print("== Checks Version Information ==\n");
  Serial.println(Esp01.ExecATGMR());

  Serial.print("\n== UART Configuration ==\n");
  Serial.println(Esp01.QueryATUARTCUR());

  Serial.print("\n== Current Wi-Fi mode ==\n");
  Serial.println(Esp01.QueryATCWMODECUR());
#endif
  Serial.print("\n== Get Current AP ==\n");
  Serial.println(Esp01.QueryATCWJAPCUR());

  Serial.print("\n== Set Configuration of SNTP ==\n");
  Serial.println(Esp01.SetATCIPSNTPCFG());

  Serial.print("\n== Get Configuration of SNTP ==\n");
  Serial.println(Esp01.QueryATCIPSNTPCFG());

  delay(1000);
  Serial.print("\n== Checks the SNTP Time ==\n");
  Serial.println(Esp01.QueryATCIPSNTPTIME());

  SntpTime=Esp01.GetSntpTime();
  setTime(SntpTime.substring(11,13).toInt(),SntpTime.substring(14,16).toInt(),SntpTime.substring(17,19).toInt(),SntpTime.substring(8,10).toInt(),SntpTime.substring(5,7).toInt(), SntpTime.substring(2,4).toInt());

  // Light
  Alarm.alarmRepeat(8,30,0, LightOn);    // 8:30 every day
  Alarm.alarmRepeat(20,30,0, LightOff);  // 20:30 every day

  // Feeder
  Alarm.alarmRepeat(10,32,0,FeederOn);   
  Alarm.alarmRepeat(10,32,3,FeederOff); 
  Alarm.alarmRepeat(20,45,0,FeederOn);   
  Alarm.alarmRepeat(20,45,3,FeederOff); 

  // Sync Time
  Alarm.alarmRepeat(7,30,0, SynSntpTime);    // 7:30 every day
  
  // LCD & Temperature
  Alarm.timerRepeat(60, Repeats);      // timer for every 60 seconds

  // HC-SR501 PIR Sensor
  Alarm.timerRepeat(2, Repeats2);      // timer for every 2 seconds

}

void loop(void)
{
  Alarm.delay(1000); // wait one second between clock display
}

void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(year());
  Serial.print("-");
  printDigits(month());
  Serial.print("-");
  printDigits(day());
  Serial.print(" ");
  printDigits(hour());
  Serial.print(":");
  printDigits(minute());
  Serial.print(":");
  printDigits(second());
  
}
void printDigits(int digits) {
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void SynSntpTime() {
  digitalClockDisplay();
  Serial.println("  [SynTime] - Sync SNTP Time");
  Serial.println();
}

// functions to be called when an alarm triggers:
void LightOn() {
  digitalClockDisplay();
  Serial.println("  [Light] - Turn Lights On");
  Serial.println();
}
void LightOff() {
  digitalClockDisplay();
  Serial.println("  [Light] - Turn Lights Off");
  Serial.println();
}
void FeederOn() {
  digitalClockDisplay();
  Serial.print("  [Feeder] Fish Feeder Start");
  Serial.println();
}
void FeederOff() {
  digitalClockDisplay();
  Serial.print("  [Feeder] Fish Feeder End");
  Serial.println();
}

void Repeats() {
  digitalClockDisplay();
  Serial.print("  60 second timer");
  Serial.println();
}

void Repeats2() {
  digitalClockDisplay();
  Serial.print("  2 second timer");
  Serial.println();
  
}


