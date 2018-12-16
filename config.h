
#ifndef _CONFIG_H
#define _CONFIG_H

#include "reg.h"

#define	LED_DATA	P2

sbit ACC_7 = ACC ^ 7;


sbit LED3= P0^3;
sbit LED2 = P0^2;
sbit LED1= P0^1;
sbit LED0 = P0^0;

sbit KEY_LEFT = P0 ^ 4;
sbit KEY_RIGHT = P0 ^ 5;
sbit KEY_UP = P0 ^ 6;
sbit KEY_DOWN = P0 ^ 7;

sbit RADIO_SDA = P1 ^ 1;
sbit RADIO_SCL = P1 ^ 2;

/* To choose Japan band or European band */
sbit BAND_SWITCH = P1 ^ 3;

sbit DQ = P1 ^ 7;
	
sbit IR = P3 ^ 3;

#define CHANNEL_VALID_FLAG		0xBEBE


#define LOW_FREQ	87.5
#define HIGH_FREQ	108.0


#define IR_CHANNEL_0	0x0
#define IR_CHANNEL_1	0x1
#define IR_CHANNEL_2	0x2
#define IR_CHANNEL_3	0x3
#define IR_CHANNEL_4	0x4
#define IR_CHANNEL_5	0x5
#define IR_CHANNEL_6	0x06
#define IR_CHANNEL_7	0x07
#define IR_CHANNEL_8	0x08
#define IR_CHANNEL_9	0x09

#define IR_MODE_SWITCH	0x078

#define IR_STANDBY_RADIO	0x088
#define IR_WAKEUP_RADIO	0x0654
#define IR_AUTO_UP_SCAN	0x077
#define IR_AUTO_DOWN_SCAN	0x044
#define IR_CHANNEL_UP			0x047
#define IR_CHANNEL_DOWN		0x0445
#define IR_FREQ_UP				0x011
#define IR_FREQ_DOWN			0x025
#define IR_DELETE_CHANNEL		0x0788
#define IR_STORE_CHANNEL		0x0456


enum WORK_MODE {
	TEMPERATURE_MODE,
	RADIO_MODE
};


enum SYMBOL {
	FREQ_ERROR
};


#define fosc 24
#define time0 9000   //定时器的定时值，太大容易闪烁，太小一直中断转换速度慢

#define TIMER_COUNT_HIGH_20MS	0x63
#define TIMER_COUNT_LOW_20MS		0xC0

/* For time 31.25ms */
#define TIMER_COUNT_HIGH	0x0B
#define TIMER_COUNT_LOW		0xDC


void delayUS(ushort us);

/*
 *延时t毫秒
 */
void delayMS(ushort ms);

#define DEBUG

#endif


