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
     * Verify ESP8266AT whether live or not. 
     *
     * Actually, this method will send command "AT" to ESP8266AT and waiting for "OK". 
     * 
     * @retval true - alive.
     * @retval false - dead.
     */
    bool kick(void);
    
    /**
     * Restart ESP8266AT by "AT+RST". 
     *
     * This method will take 3 seconds or more. 
     *
     * @retval true - success.
     * @retval false - failure.
     */
    bool restart(void);
    
    /**
     * Get the version of AT Command Set. 
     * 
     * @return the string of version. 
     */
    String getVersion(void);
	
	/**
     * Get UART Configuration of AT Command Set. 
     * 
     * @return the string of version. 
     */
    String getUartConfiguration(void);
    
	/**
     * Query Connects AP of AT Command Set. 
     * 
     * @return the string of version. 
     */
    String getCurrentAp(void);
    
    /**
     * Set operation mode to staion. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool setOprToStation(void);
    
    /**
     * Set operation mode to softap. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool setOprToSoftAP(void);
    
    /**
     * Set operation mode to station + softap. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool setOprToStationSoftAP(void);
    
    /**
     * Search available AP list and return it.
     * 
     * @return the list of available APs. 
     * @note This method will occupy a lot of memeory(hundreds of Bytes to a couple of KBytes). 
     *  Do not call this method unless you must and ensure that your board has enough memery left.
     */
    String getAPList(void);
    
    /**
     * Join in AP. 
     *
     * @param ssid - SSID of AP to join in. 
     * @param pwd - Password of AP to join in. 
     * @retval true - success.
     * @retval false - failure.
     * @note This method will take a couple of seconds. 
     */
    bool joinAP(String ssid, String pwd);
       
    /**
     * Leave AP joined before. 
     *
     * @retval true - success.
     * @retval false - failure.
     */
    bool leaveAP(void);
    
    /**
     * Set SoftAP parameters. 
     * 
     * @param ssid - SSID of SoftAP. 
     * @param pwd - PASSWORD of SoftAP. 
     * @param chl - the channel (1 - 13, default: 7). 
     * @param ecn - the way of encrypstion (0 - OPEN, 1 - WEP, 
     *  2 - WPA_PSK, 3 - WPA2_PSK, 4 - WPA_WPA2_PSK, default: 4). 
     * @note This method should not be called when station mode. 
     */
    bool setSoftAPParam(String ssid, String pwd, uint8_t chl = 7, uint8_t ecn = 4);
    
    /**
     * Get the IP list of devices connected to SoftAP. 
     * 
     * @return the list of IP.
     * @note This method should not be called when station mode. 
     */
    String getJoinedDeviceIP(void);
    
    /**
     * Get the current status of connection(UDP and TCP). 
     * 
     * @return the status. 
     */
    String getIPStatus(void);
    
    /**
     * Get the IP address of ESP8266AT. 
     *
     * @return the IP list. 
     */
    String getLocalIP(void);
    
    /**
     * Enable IP MUX(multiple connection mode). 
     *
     * In multiple connection mode, a couple of TCP and UDP communication can be builded. 
     * They can be distinguished by the identifier of TCP or UDP named mux_id. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool enableMUX(void);
    
    /**
     * Disable IP MUX(single connection mode). 
     *
     * In single connection mode, only one TCP or UDP communication can be builded. 
     * 
     * @retval true - success.
     * @retval false - failure.
     */
    bool disableMUX(void);
    

 private:

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
     * Receive a package from uart. 
     *
     * @param buffer - the buffer storing data. 
     * @param buffer_size - guess what!
     * @param data_len - the length of data actually received(maybe more than buffer_size, the remained data will be abandoned).
     * @param timeout - the duration waitting data comming.
     * @param coming_mux_id - in single connection mode, should be NULL and not NULL in multiple. 
     */
    uint32_t recvPkg(uint8_t *buffer, uint32_t buffer_size, uint32_t *data_len, uint32_t timeout, uint8_t *coming_mux_id);
    
    
    bool eAT(void);
    bool eATRST(void);
    bool eATGMR(String &version);
    bool eATUART(String &UartConfig);
	bool qATCWJAPCUR(String &CurrentAp);
	
    bool qATCWMODE(uint8_t *mode);
    bool sATCWMODE(uint8_t mode);
    bool sATCWJAP(String ssid, String pwd);
    bool eATCWLAP(String &list);
    bool eATCWQAP(void);
    bool sATCWSAP(String ssid, String pwd, uint8_t chl, uint8_t ecn);
    bool eATCWLIF(String &list);
    
    bool eATCIPSTATUS(String &list);
    bool eATCIFSR(String &list);
    bool sATCIPMUX(uint8_t mode);
    
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

