#include <SoftwareSerial.h>
#include <ESP8266AT.h>

SoftwareSerial EspSerial(10,11);
ESP8266AT Esp01(EspSerial);

void setup(void)
{
    Serial.begin(115200);

    Serial.print("== Checks Version Information ==\n");
    Serial.println(Esp01.getVersion());

}

void loop(void)
{
}
