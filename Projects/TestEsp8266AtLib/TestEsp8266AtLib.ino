#include <SoftwareSerial.h>
#include <ESP8266AT.h>

SoftwareSerial EspSerial(10, 11);
ESP8266AT Esp01(EspSerial);

void setup(void)
{
  Serial.begin(115200);

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

  delay(1000);
  Serial.print("\n== Checks the SNTP Time ==\n");
  Serial.println(Esp01.QueryATCIPSNTPTIME());

//  Esp01.setOprToStation();
//  Serial.print("\n== Get AP List ==\n");
//  Serial.println(Esp01.getAPList());
}

void loop(void)
{
  Serial.println(Esp01.GetSntpTime());
  delay (1000);
}
