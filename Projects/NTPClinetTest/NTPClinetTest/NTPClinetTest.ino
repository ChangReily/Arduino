#include <NTPClient.h>
// change next line to use with another board/shield
#include <ESP8266WiFi.h>
//#include <WiFi.h> // for WiFi shield
//#include <WiFi101.h> // for WiFi 101 shield or MKR1000
#include <WiFiUdp.h>
const char *ssid     = "NoInternet_2.4G";
const char *password = "0912841613";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

int GlobalRelayStatus = 0;
int RelayStatus = 0;
int RelayPin = 16;

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 500 );
    Serial.print ( "." );
  }

  timeClient.begin();
  timeClient.setTimeOffset(28800);

  pinMode(RelayPin, OUTPUT);   // sets the digital pin as output
  digitalWrite(RelayPin, HIGH);
}


int CalculateTimeCout(int Hours, int Minutes, int Seconds) {
  return (Hours * 60 * 60 + Minutes * 60 + Seconds);
}

void loop() {
  if (!timeClient.forceUpdate()) {
    timeClient.update();
  }
  Serial.println(timeClient.getFormattedTime());
  int hours, mins, secs, TimeCount, RelayOnTimeCount, RelayOffTimeCount;
  hours = timeClient.getHours();
  mins = timeClient.getMinutes();
  secs = timeClient.getSeconds();
  TimeCount = CalculateTimeCout(hours, mins, secs);
  Serial.print("Totaol time count:");
  Serial.println(TimeCount);
  RelayOnTimeCount = CalculateTimeCout(13, 30, 0);
  RelayOffTimeCount = CalculateTimeCout(20, 30, 0);
  if ((RelayOnTimeCount < TimeCount) and (TimeCount < RelayOffTimeCount)) {
    RelayStatus = 1;
  } else {
    RelayStatus = 0;
  }
  Serial.print("GlobalRelayStatus: ");
  Serial.println(GlobalRelayStatus);
  Serial.print("RelayStatus: ");
  Serial.println(RelayStatus);
  if (GlobalRelayStatus != RelayStatus) {
    GlobalRelayStatus = RelayStatus;
    if (GlobalRelayStatus == 1) {
      digitalWrite(RelayPin, LOW);   // sets the LED on
    } else {
      digitalWrite(RelayPin, HIGH);   // sets the LED on
    }
  }

  delay(10000);
}
