#include <SPI.h>
#include <MFRC522.h>     // 引用程式庫
#include "pitches.h"

#define RST_PIN      A0        // 讀卡機的重置腳位
#define SS_PIN       10        // 晶片選擇腳位
int duration = 100;  // 500 miliseconds
MFRC522 mfrc522(SS_PIN, RST_PIN);  // 建立MFRC522物件

void setup() {
  Serial.begin(9600);
  Serial.println("RFID reader is ready!");

  SPI.begin();
  mfrc522.PCD_Init();   // 初始化MFRC522讀卡機模組
}

void loop() {
    // 確認是否有新卡片
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {
      byte *id = mfrc522.uid.uidByte;   // 取得卡片的UID
      byte idSize = mfrc522.uid.size;   // 取得UID的長度

      Serial.print("PICC type: ");      // 顯示卡片類型
      // 根據卡片回應的SAK值（mfrc522.uid.sak）判斷卡片類型
      MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
      Serial.println(mfrc522.PICC_GetTypeName(piccType));

      Serial.print("UID Size: ");       // 顯示卡片的UID長度值
      Serial.println(idSize);
 
      for (byte i = 0; i < idSize; i++) {  // 逐一顯示UID碼
        Serial.print("id[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(id[i], HEX);       // 以16進位顯示UID值
      }
      //if (id[0]==40 && id[1]==79 && id[2]==43 && id[3]==83){
       if (id[0]==0x40 || id[0]==0x25){
        Serial.print("Get ");
        tone(8, NOTE_C6, duration);
        delay(500);
        tone(8, NOTE_C6, duration);
        delay(500);
        tone(8, NOTE_C6, duration);
        delay(500);    
      }
      Serial.println();

      mfrc522.PICC_HaltA();  // 讓卡片進入停止模式
    } 
}
