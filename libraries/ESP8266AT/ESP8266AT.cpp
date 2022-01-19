/**

 */
#include "ESP8266AT.h"


#ifdef ESP8266AT_USE_SOFTWARE_SERIAL
ESP8266AT::ESP8266AT(SoftwareSerial &uart, uint32_t baud): m_puart(&uart)
{
    // Serial.begin(115200);
    // Serial.println("Init Arduino Serial port");
    m_puart->begin(baud);
    rx_empty();
}
#else
ESP8266AT::ESP8266AT(HardwareSerial &uart, uint32_t baud): m_puart(&uart)
{
    m_puart->begin(baud);
    rx_empty();
}
#endif

bool ESP8266AT::ExecAT(void)
{
    String version;
    rx_empty();
    m_puart->println("AT");
    return recvFind("OK");
}

String ESP8266AT::ExecATGMR(void)
{
    String version;
    rx_empty();
    m_puart->println("AT+GMR");
    if (recvFindAndFilter("OK", "\r\n", "\r\nOK", version)){
        return version;
    } else {
        return "None";
    }   
}

String ESP8266AT::QueryATUARTCUR(void)
{
    String UartConfig;
    rx_empty();
    m_puart->println("AT+UART_CUR?");
    if (recvFindAndFilter("OK", "\r\n", "\r\nOK", UartConfig)){
        return UartConfig;
    } else {
        return "None";
    }   
}

String ESP8266AT::QueryATCWMODECUR(void)
{
    String WifiMode;
    rx_empty();
    m_puart->println("AT+CWMODE_CUR?");
    if (recvFindAndFilter("OK", "\r\n", "\r\n\r\nOK", WifiMode)){
        return WifiMode;
    } else {
        return "None";
    }   
}

String ESP8266AT::QueryATCWJAPCUR(void)
{
    String WifiMode;
    rx_empty();
    m_puart->println("AT+CWJAP_CUR?");
    if (recvFindAndFilter("OK", "\r\n", "\r\n\r\nOK", WifiMode)){
        return WifiMode;
    } else {
        return "None";
    }   
}

String ESP8266AT::QueryATCIPSNTPCFG(void)
{
    String SntpConfig;
    rx_empty();
    m_puart->println("AT+CIPSNTPCFG?");
    if (recvFindAndFilter("OK", "\r\n", "\r\n\r\nOK", SntpConfig)){
        return SntpConfig;
    } else {
        return "None";
    }   
}

String ESP8266AT::SetATCIPSNTPCFG(void)
{
    rx_empty();
    m_puart->println("AT+CIPSNTPCFG=1,8,\"cn.ntp.org.cn\",\"ntp.sjtu.edu.cn\",\"us.pool.ntp.org\"");
    if (recvFind("OK")){
        return "OK";
    } else {
        return "None";
    }   
}


String ESP8266AT::QueryATCIPSNTPTIME(void)
{
    String SntpTime;
    rx_empty();
    m_puart->println("AT+CIPSNTPTIME?");
    if (recvFindAndFilter("OK", "\r\n", "\n\r\nOK", SntpTime)){
        return SntpTime;
    } else {
        return "None";
    }   
}

String ESP8266AT::GetSntpTime(void)
{
    String SntpTime;
    rx_empty();
    m_puart->println("AT+CIPSNTPTIME?");
    if (recvFindAndFilter("OK", "CIPSNTPTIME:", "\n\r\nOK", SntpTime)){
        TransferTimeFormat(SntpTime);
        return SntpTime;
    } else {
        return "None";
    }   
}

bool ESP8266AT::SetATCWJAPDEF(String ssid, String pwd)
{
    String data;
    rx_empty();
    m_puart->print("AT+CWJAP_DEF=\"");
    m_puart->print(ssid);
    m_puart->print("\",\"");
    m_puart->print(pwd);
    m_puart->println("\"");
    
    data = recvString("OK", "FAIL", 10000);
    if (data.indexOf("OK") != -1) {
        return true;
    }
    return false;
}

// 
/*----------------------------------------------------------------------------*/
/* +IPD,<id>,<len>:<data> */
/* +IPD,<len>:<data> */

void ESP8266AT::TransferTimeFormat(String &SntpTime)
{
    String MonthTable[12][2]={{"Jan", "01"},
                    {"Feb", "02"},
                    {"Mar", "03"},
                    {"Apr", "04"},
                    {"May", "05"},
                    {"Jun", "06"},
                    {"Jul", "07"},
                    {"Aug", "08"},
                    {"Sep", "09"},
                    {"Oct", "10"},
                    {"Nov", "11"},
                    {"Dec", "12"}};
    for (int Index=0; Index<12; Index++){
        if (SntpTime.substring(4, 7) == MonthTable[Index][0]){
			SntpTime.replace(MonthTable[Index][0], MonthTable[Index][1]);
            break;
        }
    }
  
}

void ESP8266AT::rx_empty(void) 
{
    while(m_puart->available() > 0) {
        m_puart->read();
    }
}

String ESP8266AT::recvString(String target, uint32_t timeout)
{
    String data;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while(m_puart->available() > 0) {
            a = m_puart->read();
            if(a == '\0') continue;
            data += a;
        }
        if (data.indexOf(target) != -1) {
            break;
        }   
    }
    return data;
}

String ESP8266AT::recvString(String target1, String target2, uint32_t timeout)
{
    String data;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while(m_puart->available() > 0) {
            a = m_puart->read();
            if(a == '\0') continue;
            data += a;
        }
        if (data.indexOf(target1) != -1) {
            break;
        } else if (data.indexOf(target2) != -1) {
            break;
        }
    }
    return data;
}

String ESP8266AT::recvString(String target1, String target2, String target3, uint32_t timeout)
{
    String data;
    char a;
    unsigned long start = millis();
    while (millis() - start < timeout) {
        while(m_puart->available() > 0) {
            a = m_puart->read();
            if(a == '\0') continue;
            data += a;
        }
        if (data.indexOf(target1) != -1) {
            break;
        } else if (data.indexOf(target2) != -1) {
            break;
        } else if (data.indexOf(target3) != -1) {
            break;
        }
    }
    return data;
}

bool ESP8266AT::recvFind(String target, uint32_t timeout)
{
    String data_tmp;
    data_tmp = recvString(target, timeout);
    if (data_tmp.indexOf(target) != -1) {
        return true;
    }
    return false;
}

bool ESP8266AT::recvFindAndFilter(String target, String begin, String end, String &data, uint32_t timeout)
{
    String data_tmp;
    data_tmp = recvString(target, timeout);
    // Serial.println("==========");
    // Serial.println(data_tmp);
    // Serial.println("==========");
    if (data_tmp.indexOf(target) != -1) {
        int32_t index1 = data_tmp.indexOf(begin);
        int32_t index2 = data_tmp.indexOf(end);
        if (index1 != -1 && index2 != -1) {
            index1 += begin.length();
            data = data_tmp.substring(index1, index2);
            return true;
        }
    }
    data = "";
    return false;
}
