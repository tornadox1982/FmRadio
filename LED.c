#include "types.h"
#include "config.h"
#include "LED.h"

/*        ��ʾ����Ϊ    0       1     2     3    4      5     6     7     8     9      A     b     c     d     E     F */
static char code NUMBER[16] = {0xD7, 0x14, 0xE5, 0xB5, 0x36, 0xB3, 0xF3, 0x15, 0xF7, 0xB7, 0x77, 0xF2, 0xC3, 0xF4, 0xE3, 0x63};

static char code point = 0x8;

static char code negtive = 0x20;

//sbit  bitMask[4] = {LED0; P2^5; P2^6; P2^7};

#if 0
/*
 *��ʱt����
 */
static void delay(uint t)
{
	uint i;
	while(t--)
	{
		//ʱ��Ϊ24MHZʱ��Լ��ʱ1ms
		for(i=0; i<250; i++) 
		{ ; }
	}
}

#endif

void showChar(char numb)
{
	char neg = 0;
	char num;
	char i = 0;
		
	if (numb < 0)
	{
		num = -numb;
		neg = 1;
	}
	else
		num = numb;

	//���ȹر�ÿһλ����ʾ
	LED0 = 1;
	LED1 = 1;
	LED2 = 1;
	LED3 = 1;

	if(num == 0)
	{
		LED_DATA = NUMBER[0];
		LED0 = 0;
		delayMS(4);
		return;
	}

	//����ʾĳһλ��ʱ�򣬱���ص�����λ����ʾ������ͻ���ɲ�Ӱ
	//�°汾�н���ʾ���ּ�������ʱ���ֶ��ó������Լ�С�����С
	while(num)
	{
		LED_DATA = NUMBER[ num % 10 ];

		switch(i)
		{
		case 0:
			LED0 = 0;
			LED1 = 1;
			LED2 = 1;
			LED3 = 1;	
			break;

		case 1:
			LED0 = 1;
			LED1 = 0;
			LED2 = 1;
			LED3 = 1;
			break;

		case 2:			
			LED0 = 1;
			LED1 = 1;
			LED2 = 0;
			LED3 = 1;		
			break;

		case 3:
			LED0 = 1;
			LED1 = 1;
			LED2 = 1;
			LED3 = 0;	
			break;
		}

		num = num / 10;
		i++;			
		delayMS(4);		
	}

	//�����Ѿ�������λ�����޷���ʾ����
	if (neg == 1 && i != 4)
	{	
		LED_DATA = negtive;		
		
		switch(i)
		{
			case 0:
				LED0 = 0;
				LED1 = 1;
				LED2 = 1;
				LED3 = 1;
				break;
			case 1:
				LED0 = 1;
				LED1 = 0;
				LED2 = 1;
				LED3 = 1;
				break;
			case 2:
				LED0 = 1;
				LED1 = 1;
				LED2 = 0;
				LED3 = 1;
				break;
			case 3:
				LED0 = 1;
				LED1 = 1;
				LED2 = 1;
				LED3 = 0;
				break;

			default:
				return;
		}

		delayMS(3);
	}

}

void showInt(int numb)
{
	char neg = 0;
	int num;
	char i = 0;
		
	if (numb < 0)
	{
		num = -numb;
		neg = 1;
	}
	else
		num = numb;

	//���ȹر�ÿһλ����ʾ
	LED0 = 1;
	LED1 = 1;
	LED2 = 1;
	LED3 = 1;

	if(num == 0)
	{
		LED_DATA = NUMBER[0];
		LED0 = 0;
		delayMS(4);
		return;
	}

	//����ʾĳһλ��ʱ�򣬱���ص�����λ����ʾ������ͻ���ɲ�Ӱ
	//�°汾�н���ʾ���ּ�������ʱ���ֶ��ó������Լ�С�����С
	while(num)
	{
		LED_DATA = NUMBER[ num % 10 ];

		switch(i)
		{
		case 0:
			LED0 = 0;
			LED1 = 1;
			LED2 = 1;
			LED3 = 1;	
			break;

		case 1:
			LED0 = 1;
			LED1 = 0;
			LED2 = 1;
			LED3 = 1;
			break;

		case 2:			
			LED0 = 1;
			LED1 = 1;
			LED2 = 0;
			LED3 = 1;		
			break;

		case 3:
			LED0 = 1;
			LED1 = 1;
			LED2 = 1;
			LED3 = 0;	
			break;
		}

		num = num / 10;
		i++;			
		delayMS(4);		
	}

	//�����Ѿ�������λ�����޷���ʾ����
	if (neg == 1 && i != 4)
	{	
		LED_DATA = negtive;		
		
		switch(i)
		{
			case 0:
				LED0 = 0;
				LED1 = 1;
				LED2 = 1;
				LED3 = 1;
				break;
			case 1:
				LED0 = 1;
				LED1 = 0;
				LED2 = 1;
				LED3 = 1;
				break;
			case 2:
				LED0 = 1;
				LED1 = 1;
				LED2 = 0;
				LED3 = 1;
				break;
			case 3:
				LED0 = 1;
				LED1 = 1;
				LED2 = 1;
				LED3 = 0;
				break;

			default:
				return;
		}

		delayMS(4);
	}

}

void ledShowFloat(float fl) reentrant
{
	
	char neg = 0;
	int num ;	
	char i = 0;
	
	num =  (int)(fl * 10);
		
	if (num < 0)
	{
		num = -num;
		neg = 1;
	}

	//���ȹر�ÿһλ����ʾ
	LED0 = 1;
	LED1 = 1;
	LED2 = 1;
	LED3 = 1;

	while(num)
	{
		LED_DATA = NUMBER[ num % 10 ];

		switch(i)
		{
			case 0:
				
				LED0 = 0;
				LED1 = 1;
				LED2 = 1;
				LED3 = 1;	
				break;
	
			case 1:
				LED_DATA  |= point;	
				LED0 = 1;
				LED1 = 0;
				LED2 = 1;
				LED3 = 1;
				break;
	
			case 2:			
				LED0 = 1;
				LED1 = 1;
				LED2 = 0;
				LED3 = 1;		
				break;
	
			case 3:
				LED0 = 1;
				LED1 = 1;
				LED2 = 1;
				LED3 = 0;	
				break;
		}

		num = num / 10;
		i++;			
		delayMS(80);		
			
	}

	//�����Ѿ�������λ�����޷���ʾ����
	if (neg == 1 && i < 3)
	{	
		LED_DATA = negtive;		
		
		switch(i)
		{
			case 0:
				LED0 = 0;
				LED1 = 1;
				LED2 = 1;
				LED3 = 1;
				break;

			case 1:
				LED0 = 1;
				LED1 = 0;
				LED2 = 1;
				LED3 = 1;
				break;

			case 2:
				LED0 = 1;
				LED1 = 1;
				LED2 = 0;
				LED3 = 1;
				break;

			case 3:
				LED0 = 1;
				LED1 = 1;
				LED2 = 1;
				LED3 = 0;
				break;

			default:
				return;
		}

		delayMS(4);
	}	
}


void showHex(uint num)
{
	char i = 0;

	//���ȹر�ÿһλ����ʾ
	LED0 = 1;
	LED1 = 1;
	LED2 = 1;
	LED3 = 1;

	if(num == 0)
	{
		LED_DATA = NUMBER[0];
		LED0 = 0;
		delayMS(4);
		return;
	}

	//����ʾĳһλ��ʱ�򣬱���ص�����λ����ʾ������ͻ���ɲ�Ӱ
	//�°汾�н���ʾ���ּ�������ʱ���ֶ��ó������Լ�С�����С
	while(num)
	{
		LED_DATA = NUMBER[ num % 16 ];

		switch(i)
		{
		case 0:
			LED0 = 0;
			LED1 = 1;
			LED2 = 1;
			LED3 = 1;	
			break;

		case 1:
			LED0 = 1;
			LED1 = 0;
			LED2 = 1;
			LED3 = 1;
			break;

		case 2:			
			LED0 = 1;
			LED1 = 1;
			LED2 = 0;
			LED3 = 1;		
			break;

		case 3:
			LED0 = 1;
			LED1 = 1;
			LED2 = 1;
			LED3 = 0;	
			break;
		}

		num = num / 16;
		i++;			
		delayMS(4);		
	}

}
