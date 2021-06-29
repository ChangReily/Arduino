#include "pitches.h"

// notes in the melody:
int melody[] = {
  NOTE_C5, NOTE_D5, NOTE_E5, NOTE_F5, NOTE_G5, NOTE_A5, NOTE_B5, NOTE_C6};
int duration = 500;  // 500 miliseconds

void setup() {

}

void loop() {  
  for (int thisNote = 0; thisNote < 8; thisNote++) {
    // 在 pin8 上輸出聲音，每個音階響 0.5 秒
    tone(8, melody[thisNote], duration);
    
     // 間隔一段時間後再播放下一個音階
    delay(1000);
  }
  
  // 兩秒後重新播放 
  delay(2000);
}

