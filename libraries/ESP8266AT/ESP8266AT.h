/**
 * @file ESP8266AT.h
 * @brief The definition of class ESP8266AT. 
 * @author Wu Pengfei<pengfei.wu@itead.cc> 
 * @date 2015.02
 * 
 * @par Copyright:
 * Copyright (c) 2015 ITEAD Intelligent Systems Co., Ltd. \n\n
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version. \n\n
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef __ESP8266AT_H__
#define __ESP8266AT_H__

#include "Arduino.h"


#define ESP8266AT_USE_SOFTWARE_SERIAL


#ifdef ESP8266AT_USE_SOFTWARE_SERIAL
#include "SoftwareSerial.h"
#endif


/**
 * Provide an easy-to-use way to manipulate ESP8266AT. 
 */
class ESP8266AT {
 public:

#ifdef ESP8266AT_USE_SOFTWARE_SERIAL
    /*
     * Constuctor. 
     *
     * @param uart - an reference of SoftwareSerial object. 
     * @param baud - the buad rate to communicate with ESP8266AT(default:9600). 
     *
     * @warning parameter baud depends on the AT firmware. 9600 is an common value. 
     */
    ESP8266AT(SoftwareSerial &uart, uint32_t baud = 9600);
#else /* HardwareSerial */
    /*
     * Constuctor. 
     *
     * @param uart - an reference of HardwareSerial object. 
     * @param baud - the buad rate to communicate with ESP8266AT(default:9600). 
     *
     * @warning parameter baud depends on the AT firmware. 9600 is an common value. 
     */
    ESP8266AT(HardwareSerial &uart, uint32_t baud = 9600);
#endif
    
    /**
     * Get the version of AT Command Set. 
     * 
     * @return the string. 
     */
    String ExecATGMR(void);
    
	/**
     * Get Current UART Configuration of AT Command Set. 
     * 
     * @return the string. 
     */
    String QueryATUARTCUR(void);
	
	/**
     * Get Current Wi-Fi mode of AT Command Set. 
     * 
     * @return the string. 
     */
    String QueryATCWMODECUR(void);
	
	/**
     * Get Current Wi-Fi mode of AT Command Set. 
     * 
     * @return the string. 
     */
    String QueryATCWJAPCUR(void);
	
	/**
     * Get the SNTP Configuration of AT Command Set. 
     * 
     * @return the string. 
     */
    String QueryATCIPSNTPCFG(void);	
	
	/**
     * Set the SNTP Configuration of AT Command Set. 
     * 
     * @return the string. 
     */
    String SetATCIPSNTPCFG(void);	
	
	/**
     * Checks the SNTP Time of AT Command Set. 
     * 
     * @return the string. 
     */
    String QueryATCIPSNTPTIME(void);
	
	
	
	/**
     * Get the SNTP Time
     * 
     * @return the string. 
     */
    String GetSntpTime(void);
 private:

    /* 
    * Transfer SNTP Time String Format.
    /
    void TransferTimeFormat(String &SntpTime);

    /* 
     * Empty the buffer or UART RX.
     */
    void rx_empty(void);
 
    /* 
     * Recvive data from uart. Return all received data if target found or timeout. 
     */
    String recvString(String target, uint32_t timeout = 1000);
    
    /* 
     * Recvive data from uart. Return all received data if one of target1 and target2 found or timeout. 
     */
    String recvString(String target1, String target2, uint32_t timeout = 1000);
    
    /* 
     * Recvive data from uart. Return all received data if one of target1, target2 and target3 found or timeout. 
     */
    String recvString(String target1, String target2, String target3, uint32_t timeout = 1000);
    
    /* 
     * Recvive data from uart and search first target. Return true if target found, false for timeout.
     */
    bool recvFind(String target, uint32_t timeout = 1000);
    
    /* 
     * Recvive data from uart and search first target and cut out the substring between begin and end(excluding begin and end self). 
     * Return true if target found, false for timeout.
     */
    bool recvFindAndFilter(String target, String begin, String end, String &data, uint32_t timeout = 1000);
    
    
    /*
     * +IPD,len:data
     * +IPD,id,len:data
     */
    
#ifdef ESP8266AT_USE_SOFTWARE_SERIAL
    SoftwareSerial *m_puart; /* The UART to communicate with ESP8266AT */
#else
    HardwareSerial *m_puart; /* The UART to communicate with ESP8266AT */
#endif
};

#endif /* #ifndef __ESP8266AT_H__ */

