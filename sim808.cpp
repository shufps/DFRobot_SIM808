/*
 * sim808.cpp
 * A library for DFRobot's SIM808 GPS/GPRS/GSM Shield
 *
 * @copyright	[DFRobot](http://www.dfrobot.com), 2016
 *
 * @author [Jason](jason.ling@dfrobot.com)
 * @version  V1.0
 * @date  2016-09-23
 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <string.h>

#include "sim808.h"
#include "usart.h"
#include "Timer.h"

//SoftwareSerial *serialSIM808 = NULL;

extern volatile uint32_t systick;
extern Timer timer;

/*
void  sim808_init(void * uart_device, uint32_t baud)
{
    serialSIM808 = (SoftwareSerial*)uart_device;
	  serialSIM808->begin(baud);
}

*/

void  sim808_init()
{
}


int sim808_check_readable()
{
    return USART2_isAvailable();
}

uint32_t millis() {
	return systick;
}

void delay(uint32_t ms) {
	timer.sleep(ms);
}

int sim808_wait_readable (int wait_time)
{
    unsigned long timerStart;
    int dataLen = 0;
    timerStart = millis();
    while((unsigned long) (millis() - timerStart) > wait_time * 1000UL) {
        delay(500);
        dataLen = sim808_check_readable();
        if(dataLen > 0){
            break;
        }
    }
    return dataLen;
}

void sim808_flush_serial()
{
    while(sim808_check_readable()){
    	USART2_readChar();
    }
}

void sim808_read_buffer(char *buffer, int count, unsigned int timeout, unsigned int chartimeout)
{
    int i = 0;
    unsigned long timerStart, prevChar;
    timerStart = millis();
    prevChar = 0;
    while(1) {
        while (sim808_check_readable()) {
            char c = USART2_readChar();
            prevChar = millis();
            buffer[i++] = c;
            if(i >= count)break;
        }
        if(i >= count)break;
        if ((unsigned long) (millis() - timerStart) > timeout * 1000UL) {
            break;
        }
        //If interchar Timeout => return FALSE. So we can return sooner from this function. Not DO it if we dont recieve at least one char (prevChar <> 0)
        if (((unsigned long) (millis() - prevChar) > chartimeout) && (prevChar != 0)) {
            break;
        }
    }
}

void sim808_clean_buffer(char *buffer, int count)
{
    for(int i=0; i < count; i++) {
        buffer[i] = '\0';
    }
}

//HACERR quitar esta funcion ?
void sim808_send_byte(uint8_t data)
{
	USART2_Write(&data, 1);
}

void sim808_send_char(const char c)
{
	USART2_Write((const uint8_t*) &c, 1);
}

void sim808_send_cmd(const char* cmd)
{
	USART2_Write((const uint8_t*) cmd, strlen(cmd));
}




bool sim808_send_AT(void)
{
    return sim808_check_with_cmd("AT\r\n","OK",CMD);
}

void sim808_send_End_Mark(void)
{
    sim808_send_byte((char)26);
}

bool sim808_wait_for_resp(const char* resp, DataType type, unsigned int timeout, unsigned int chartimeout)
{
    int len = strlen(resp);
    int sum = 0;
    unsigned long timerStart, prevChar;    //prevChar is the time when the previous Char has been read.
    timerStart = millis();
    prevChar = 0;

    while(1) {
        if(sim808_check_readable()) {
            char c = USART2_readChar();
            prevChar = millis();
            sum = (c==resp[sum]) ? sum+1 : 0;
            if(sum == len)break;
        }
        if ((unsigned long) (millis() - timerStart) > timeout * 1000UL) {
            return false;
        }
        //If interchar Timeout => return FALSE. So we can return sooner from this function.
        if (((unsigned long) (millis() - prevChar) > chartimeout) && (prevChar != 0)) {
            return false;
        }

    }
    //If is a CMD, we will finish to read buffer.
    if(type == CMD) sim808_flush_serial();
    return true;
}


bool sim808_check_with_cmd(const char* cmd, const char *resp, DataType type, unsigned int timeout, unsigned int chartimeout)
{
    sim808_send_cmd(cmd);
    return sim808_wait_for_resp(resp,type,timeout,chartimeout);
}

