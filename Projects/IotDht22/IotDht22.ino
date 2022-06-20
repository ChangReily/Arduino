//---- Setup update frequency -----//
const long TEMP_updateInterval = 10000; // How long to change temp and update, 10000 = 10 sec
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
    //Start DH11 sensor
    dht.begin();
    
    // Optionnal functionnalities of EspMQTTClient :
    client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
    //client.enableHTTPWebUpdater();  // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
    //client.enableLastWillMessage("TestClient/lastwill", "I am going offline"); // You can activate the retain flag by setting the third parameter to true
}
 
// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
    // Subscribe to "mytopic/test" and display received message to Serial
    client.subscribe("message/hello", [](const String &payload)
                     { Serial.println(payload); });
 
    // Subscribe to "mytopic/wildcardtest/#" and display received message to Serial
    //client.subscribe("mytopic/wildcardtest/#", [](const String &topic, const String &payload)
    //                 { Serial.println("(From wildcard) topic: " + topic + ", payload: " + payload); });
 
    // Publish a message to "mytopic/test"
    client.publish("studio/temp1", "Let's go!"); // You can activate the retain flag by setting the third parameter to true
 
    // Execute delayed instructions
    //client.executeDelayed(5 * 1000, []()
    //                      { client.publish("mytopic/wildcardtest/test123", "This is a message sent 5 seconds later"); });
}
 
 
 
void loop()
{
 
    TEMP_currentMillis = millis();
    if(TEMP_currentMillis - TEMP_previousMillis >= TEMP_updateInterval){
        TEMP_previousMillis = TEMP_currentMillis;
 
        // Reading temperature or humidity takes about 250 milliseconds!
        // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
        float humidity = dht.readHumidity();
        float temperature = dht.readTemperature();
        // Read temperature as Fahrenheit (isFahrenheit = true)
        //float f = dht.readTemperature(true);
 
        // Check if any reads failed and exit early (to try again).
        if (isnan(humidity) || isnan(temperature))
        {
            Serial.println(F("Failed to read from DHT sensor!"));
            return;
        }
 
        String tmp = "Current Temperature: ";
        String hud = "Current Humidity: ";
        Serial.print(tmp);
        Serial.println(String(temperature));
        Serial.print(hud);
        Serial.println(String(humidity));
        //Publish
        client.publish("livingroom/temp1", String(temperature));
        client.publish("livingroom/humd1", String(humidity));
 
        // Here is how to subscribe
        //client.subscribe("message/hello", [](const String &payload) { Serial.println(payload); });
    }
 
    client.loop();
}


