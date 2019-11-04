#include <SoftwareSerial.h>
#include <ESP8266AT.h>

SoftwareSerial EspSerial(10,11);
ESP8266AT Esp01(EspSerial);

void setup(void)
{
    Serial.begin(115200);

    Serial.print("== Checks Version Information ==\n");
    Serial.println(Esp01.getVersion());
    
    Serial.print("\n== UART Configuration ==\n");
    Serial.println(Esp01.getUartConfiguration());

    Serial.print("\n== Get Current AP ==\n");
    Serial.println(Esp01.getCurrentAp());

//    Esp01.setOprToStation();
//
//    Serial.print("\n== Get AP List ==\n");
//    Serial.println(Esp01.getAPList());
}

void loop(void)
{
}
