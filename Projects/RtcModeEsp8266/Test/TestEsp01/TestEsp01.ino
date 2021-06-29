// Change the defination(#define _SS_MAX_RX_BUFF 255) under
// C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\SoftwareSerial\src\SoftwareSerial.h
// for more RX buffer
#include <SoftwareSerial.h>

SoftwareSerial sSerial(10, 11); //(RX,TX) 與 ESP8266 介接的軟體串列埠

String ssid = "PPAP"; //無線基地台識別
String pwd = "0912841613"; //無線基地台密碼

void setup() {
  sSerial.begin(9600);  //設定軟體序列埠速率 (to ESP8266)
  Serial.begin(115200);  //設定軟體序列埠速率 (to PC)
  Serial.println("*** SoftSerial connection to ESP8266 ***");
  Serial.println("Firmware version : " + get_version());
  Serial.println("UART Setting: \n" + get_uart());
//    Serial.println("Mode : " + get_mode());
//    Serial.println("Set Mode=1 : " + set_mode(1));
//    Serial.println("Mode : " + get_mode());
//  
//    Serial.println("Mux : " + get_mux());
//    Serial.println("Set Mux=0 : " + set_mux(0));
//    Serial.println("Mux : " + get_mux());
  
    Serial.println("Get AP : " + get_ap());
//    Serial.println("Quit AP : " + quit_ap());
  // Serial.println("Get AP : " + get_ap());
  // Serial.println("Get IP : " + get_ip());

  // Serial.println("Joint AP : " + joint_ap(ssid, pwd));
  // Serial.println("Get AP : " + get_ap());
  // Serial.println("Get IP : " + get_ip());

  // Serial.println("Connect Google : " + start_tcp("www.google.com",80));
  // Serial.println("Send GET : " + send_data("GET /"));
  // Serial.println("Connect Thingspeak : " + start_tcp("184.106.153.149",80));
  // Serial.println("Send GET : " + send_data("GET /update?api_key=NO5N8C7T2KINFCQE&field1=28.00&field2=82.40&field3=81.00"));
}

void loop() {
  if (sSerial.available()) {  //若軟體串列埠 RX 有收到來自 ESP8266 的回應字元
    Serial.write(sSerial.read());  //在串列埠監控視窗顯示 ESP8266 的回應字元
  }
  if (Serial.available()) {  //若串列埠 RX 有收到來自 PC 的 AT 指令字元 (USB TX)
    sSerial.write(Serial.read());  //將 PC 的傳來的字元傳給 ESP8266
  }
}

String get_ESP8266_response() {  //取得 ESP8266 的回應字串
  String str = ""; //儲存接收到的回應字串
  char c;  //儲存接收到的回應字元
  while (sSerial.available() > 0) { //若軟體序列埠接收緩衝器還有資料
    c = sSerial.read(); //必須放入宣告為 char 之變數 (才會轉成字元)
    str.concat(c);  //串接回應字元
    delay(10);  //務必要延遲, 否則太快
  }
//  Serial.println(str);
  str.trim();  //去除頭尾空白字元
  return str;
}

String get_version() {
  sSerial.println("AT+GMR");  //取得韌體版本
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") == -1) {
    return "NG";
  }
  else {
    return str.substring(str.indexOf("\r\n"), str.indexOf("OK"));
  }
}

String get_uart() {
  sSerial.println("AT+UART?");  //取得傳送速率
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") == -1) {
    return "NG";
  }
  else {
    return str.substring(str.indexOf(":") + 1, str.indexOf("OK"));
  }
}

String get_ip() {
  sSerial.println("AT+CIFSR");  //取得 ESP8266 IP
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") == -1) {
    return "NG";
  }
  else {
    return str.substring(0, str.indexOf("\r\n"));
  }
}

String get_mode() {
  sSerial.println("AT+CWMODE?");  //取得工作模式
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") != -1) {
    return str.substring(str.indexOf(":") + 1, str.indexOf("\r\n\r\nOK"));
  }
  else {
    return "NG";
  }
}

String set_mode(byte mode) {
  sSerial.println("AT+CWMODE=" + String(mode));  //設定工作模式
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") != -1 || str.indexOf("no change") != -1) {
    return "OK";
  }
  else {
    return "NG";
  }
}

String get_mux() {
  sSerial.println("AT+CIPMUX?");  //取得連線模式
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") != -1) {
    return str.substring(str.indexOf(":") + 1, str.indexOf("\r\n"));
  }
  else {
    return "NG";
  }
}

String set_mux(byte mux) {  //0=single, 1=multiple
  sSerial.println("AT+CIPMUX=" + String(mux));  //設定連線模式
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") != -1) {
    return "OK";
  }
  else {
    return "NG";
  }
}

String get_ap() {
  sSerial.println("AT+CWJAP_CUR?");  //取得連線之AP
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") != -1) {
    if (str.indexOf("No AP") != -1) {
      return "No AP";  
    } else {
      return str.substring(str.indexOf(":") + 1, str.indexOf("\r\nOK"));
    } 
  }
  else {
    return "NG";
  }
}

String joint_ap(String ssid, String pwd) {
  sSerial.println("AT+CWJAP=" + ssid + "," + pwd + "");  //連線
  sSerial.flush();  //等待序列埠傳送完畢
  delay(7000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") != -1) {
    return "OK";
  }
  else {
    return "NG";
  }
}

String quit_ap() {
  sSerial.println("AT+CWQAP");  //離線
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") != -1) {
    return "OK";
  }
  else {
    return "NG";
  }
}

String start_tcp(String address, byte port) {
  sSerial.println("AT+CIPSTART=\"TCP\",\"" + address + "\"," + String(port));
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("Linked") != -1) {
    return "OK";
  }
  else {
    return "NG";
  }
}

String send_data(String s) {
  String s1 = s + "\r\n"; //務必加上跳行
  sSerial.println("AT+CIPSEND=" + String(s1.length()));
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf(">") != -1) {  //收到 > 開始傳送資料
    sSerial.println(s1); //傳送資料
    sSerial.flush();  //等待序列埠傳送完畢
    delay(7000);
    str = get_ESP8266_response(); //取得 ESP8266 回應字串
    if (str.indexOf("+IPD") != -1) {
      return "OK"; //傳送成功會自動拆線
    }
    else {  //傳送不成功須自行拆線
      close_ip();  //關閉 IP 連線
      return "NG";
    }
  }
  else {  //傳送不成功須自行拆線
    close_ip();  //關閉 IP 連線
    return "NG";
  }
}

String close_ip() {
  sSerial.println("AT+CIPCLOSE");  //關閉 IP 連線
  sSerial.flush();  //等待序列埠傳送完畢
  delay(1000);
  String str = get_ESP8266_response(); //取得 ESP8266 回應字串
  if (str.indexOf("OK") != -1) {
    return "OK";
  }
  else {
    return "NG";
  }
}
