//---- Setup update frequency -----//
const long TEMP_updateInterval = 5000; // How long to change temp and update, 10000 = 10 sec
unsigned long TEMP_currentMillis = 0;
unsigned long TEMP_previousMillis = 0; // store last time temp update
 
#include "DHT.h"
#define DHTPIN 2  //ESP32 pin 23
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE); // Initialize DHT sensor
 
/*
  SimpleMQTTClient.ino
  The purpose of this exemple is to illustrate a simple handling of MQTT and Wifi connection.
  Once it connects successfully to a Wifi network and a MQTT broker, it subscribe to a topic and send a message to it.
  It will also send a message delayed 5 seconds later.
*/
 
#include "EspMQTTClient.h"
 
EspMQTTClient client(
    "NoInternet_2.4G",     // Wi-Fi AP name
    "0912841613", // Wi-Fi Password
    "192.168.0.98", // MQTT Broker server ip
    "",                  // Broker user name; Can be omitted if not needed
    "",                  // Broker password; Can be omitted if not needed
    "ESP32_Client",      // Client name that uniquely identify your device
    1883                 // The MQTT port, default to 1883. this line can be omitted
);
 
void setup()
{
    Serial.begin(115200);
    
    dht.begin();
    
    client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
}
 
// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
   // Publish a message to "mytopic/test"
    client.publish("426-1/debug", "Let's go!"); // You can activate the retain flag by setting the third parameter to true
 
}

void loop()
{
 
    TEMP_currentMillis = millis();
    if(TEMP_currentMillis - TEMP_previousMillis >= 3000){
        TEMP_previousMillis = TEMP_currentMillis;
 
        // Reading temperature or humidity takes about 250 milliseconds!
        // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
        float humidity = dht.readHumidity()-10;
        float temperature = dht.readTemperature();
        
        // Check if any reads failed and exit early (to try again).
        if (isnan(humidity) || isnan(temperature))
        {
            client.publish("426-1/debug", "Failed to read from DHT sensor!");
            return;
        }
 
        String tmp = "Current Temperature: ";
        String hud = "Current Humidity: ";
        Serial.print(tmp);
        Serial.println(String(temperature));
        Serial.print(hud);
        Serial.println(String(humidity));
        //Publish
        client.publish("426-1/temp1", String(temperature));
        client.publish("426-1/humd1", String(humidity));
        String MsgStr = "";
        MsgStr = MsgStr+"{\"temp\":" + temperature + ", "
                        +"\"humd\":" + humidity + "}";
        client.publish("426-1/Dht22Data", MsgStr);

    }
 
    client.loop();
}


