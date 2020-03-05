/*
  Basic code for using Maxim MAX7219/MAX7221 with Arduino.

  Wire the Arduino and the MAX7219/MAX7221 together as follows:

  | Arduino   | MAX7219/MAX7221 |
  | --------- | --------------- |
  | MOSI (11) | DIN (1)         |
  | SCK (13)  | CLK (13)        |
  | SS (10)*  | LOAD/CS (12)    |

    * - This should match the LOAD_PIN constant defined below.
  
  For the rest of the wiring follow the wiring diagram found in the datasheet.
  
  Datasheet: http://datasheets.maximintegrated.com/en/ds/MAX7219-MAX7221.pdf

  Author:  Nicholas Dobie <nick@nickdobie.com>
  Date:    30 December 2013
  License: WTFPL (http://www.wtfpl.net/)
*/
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <SPI.h>


#define  SET_MINUTE 3
#define  SET_HOUR   1

// What pin on the Arduino connects to the LOAD/CS pin on the MAX7219/MAX7221
#define LOAD_PIN 10

uint8_t      Relay_Switch;
tmElements_t tm;

/**
 * Transfers data to a MAX7219/MAX7221 register.
 * 
 * @param address The register to load data into
 * @param value   Value to store in the register
 */
void Max7219Transfer(uint8_t address, uint8_t value) {

  // Ensure LOAD/CS is LOW
  digitalWrite(LOAD_PIN, LOW);

  // Send the register address
  SPI.transfer(address);

  // Send the value
  SPI.transfer(value);

  // Tell chip to load in data
  digitalWrite(LOAD_PIN, HIGH);
}
  

void setup() {
  Serial.begin(9600);

  //--------------------------------------------------------------------------------
  // Set load pin to output
  pinMode(LOAD_PIN, OUTPUT);
  // Reverse the SPI transfer to send the MSB first  
  SPI.setBitOrder(MSBFIRST);
  // Start SPI
  SPI.begin();
  // All LED segments should light up
  Max7219Transfer(0x0F, 0x01);
  delay(1000);
  Max7219Transfer(0x0F, 0x00);
  // Enable mode B
  Max7219Transfer(0x09, 0xFF);
  // Use lowest intensity
  Max7219Transfer(0x0A, 0x02);
  // Only scan one digit
  Max7219Transfer(0x0B, 0x03);
  // Turn on chip
  Max7219Transfer(0x0C, 0x01);

  //--------------------------------------------------------------------------------
  // Initialize Timer
  setSyncProvider(RTC.get);
  setSyncInterval(1800);
  // Serial.println("123213");
  Relay_Switch=0;
  pinMode(8, INPUT_PULLUP);                      // button1 is connected to pin 8
  pinMode(9, INPUT_PULLUP);                      // button2 is connected to pin 9

}

void loop() {
  uint8_t MinData;
  uint8_t HourData;

  // Loop through each code

  if(!digitalRead(8)){                           // If button (pin #8) is pressed
    RTC.read(tm);
    tm.Hour   = edit(SET_HOUR, tm.Hour);
    tm.Minute = edit(SET_MINUTE, tm.Minute);
    // Serial.println(String(tm.Hour)+":"+String(tm.Minute));
    RTC.write(tm);
    setTime(makeTime(tm));
  }

  HourData=hour();
  MinData=minute();
 
  Max7219Transfer(SET_HOUR,   HourData/10);
  Max7219Transfer(SET_HOUR+1, HourData%10);

  Max7219Transfer(SET_MINUTE,   (MinData/10)|0x80);
  Max7219Transfer(SET_MINUTE+1, (MinData%10)|0x80);
  delay(500);
  Max7219Transfer(SET_MINUTE,   MinData/10);
  Max7219Transfer(SET_MINUTE+1, MinData%10);
  delay(500);
  
  if ((MinData/10)==0){
    if (Relay_Switch==0){
      Relay_Switch=1;
      // Serial.println(String(MinData/10)+String(MinData%10)+" ON");
    }
  } else {
    if (Relay_Switch==1){
      Relay_Switch=0;
      // Serial.println(String(MinData/10)+String(MinData%10)+" OFF");
    }
  }
  
}

byte edit(uint8_t SetIndex, uint8_t parameter){
  while(!digitalRead(8));                        // Wait until button (pin #8) released
  while(true){
    while(!digitalRead(9)){                      // If button (pin #9) is pressed
      parameter++;
      if(SetIndex == SET_MINUTE && parameter > 59)               // If hours > 23 ==> hours = 0
        parameter = 0;
      if(SetIndex == SET_HOUR && parameter > 23)               // If minutes > 59 ==> minutes = 0
        parameter = 0;
      delay(200);                                // Wait 200ms
      Max7219Transfer(SetIndex, parameter/10);
      Max7219Transfer(SetIndex+1, parameter%10);
    }
    Max7219Transfer(SetIndex, 0x0F);
    Max7219Transfer(SetIndex+1, 0x0F);
    delay(250);
    Max7219Transfer(SetIndex, parameter/10);
    Max7219Transfer(SetIndex+1, parameter%10);
    delay(250);
    if(!digitalRead(8)){                         // If button (pin #8) is pressed
      return parameter;                          // Return parameter value and exit
    }
  }
}