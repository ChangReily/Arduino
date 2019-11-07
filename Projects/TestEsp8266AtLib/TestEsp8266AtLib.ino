#include <SoftwareSerial.h>
#include <ESP8266AT.h>

#include <TimeLib.h>
#include <TimeAlarms.h>

SoftwareSerial EspSerial(10, 11);
ESP8266AT Esp01(EspSerial);

#define DEBUG_8266 0

void setup(void)
{
  String SntpTime;
  Serial.begin(115200);

  while (!Esp01.ExecAT()) {
#if DEBUG_8266
    Serial.println("Exec AT");
#endif    
    delay(2000);
  }
  while (Esp01.SetATCIPSNTPCFG() == "None") {
#if DEBUG_8266
    Serial.println("Set SNTP Config");
#endif
    delay(2000);
  }
//  while (Esp01.QueryATCWJAPCUR() == "No AP") {
//#if DEBUG_8266
//    Serial.println("No AP");
//#endif
//    delay(2000);
//  }
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
  SntpTime = Esp01.GetSntpTime();
  setTime(SntpTime.substring(11, 13).toInt(), SntpTime.substring(14, 16).toInt(), SntpTime.substring(17, 19).toInt(), SntpTime.substring(8, 10).toInt(), SntpTime.substring(5, 7).toInt(), SntpTime.substring(0, 4).toInt());

  // Light
  Alarm.alarmRepeat(8, 30, 0, LightOn);  // 8:30 every day
  Alarm.alarmRepeat(20, 30, 0, LightOff); // 20:30 every day

  // Feeder
  Alarm.alarmRepeat(8, 45, 0, FeederOn);
  Alarm.alarmRepeat(8, 45, 3, FeederOff);
  Alarm.alarmRepeat(20, 45, 0, FeederOn);
  Alarm.alarmRepeat(20, 45, 3, FeederOff);

  // Sync Time
  Alarm.alarmRepeat(7, 30, 0, SynSntpTime);  // 7:30 every day

  // LCD & Temperature
  Alarm.timerRepeat(60, Repeats);      // timer for every 60 seconds

  // HC-SR501 PIR Sensor
  Alarm.timerRepeat(2, Repeats2);      // timer for every 2 seconds

}

void loop(void)
{
  //Serial.println("???");
  Alarm.delay(1000); // wait one second between clock display
}

void digitalClockDisplay() {
  // digital clock display of the time
//  Serial.print(year());
//  Serial.print("-");
//  printDigits(month());
//  Serial.print("-");
//  printDigits(day());
//  Serial.print(" ");
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
}

// functions to be called when an alarm triggers:
void LightOn() {
  digitalClockDisplay();
  Serial.println("  [Light] - Turn Lights On");
}
void LightOff() {
  digitalClockDisplay();
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

void Repeats() {
  String SntpTime;
  digitalClockDisplay();
  Serial.println("  60 second timer");
  if (year()==1970){
    SntpTime = Esp01.GetSntpTime();
    Serial.println(SntpTime);
    if (SntpTime.substring(0, 4)!="1970") { 
    setTime(SntpTime.substring(11, 13).toInt(), SntpTime.substring(14, 16).toInt(), SntpTime.substring(17, 19).toInt(), SntpTime.substring(8, 10).toInt(), SntpTime.substring(5, 7).toInt(), SntpTime.substring(0, 4).toInt());
    }
  }
}

void Repeats2() {
  digitalClockDisplay();
  Serial.println("  2 second timer");
}


