
#include "types.h"
#include "config.h"


/* 每次操作后都要把RADIO_SCL拉为低电平, RADIO_SCL在高电平的时候如果数据位有变化，则
	预示着是控制位，数据位只有在RADIO_SCL低电平时才能改变
*/


/* 起始条件  RADIO_SCL 线是高电平时,RADIO_SDA线从高电平向低电平切换*/
void iicStart()
{
	/* IIC总线空闲时都是高电平 */
	RADIO_SDA = 1;
	RADIO_SCL = 1;

	delayUS(5);

	RADIO_SDA = 0;

	delayUS(5);

	RADIO_SCL = 0;
}

/* 终止条件  RADIO_SCL 线是高电平时,RADIO_SDA线从低电平向高电平切换*/
void iicStop()
{
	RADIO_SDA = 0;
	RADIO_SCL = 1;

	delayUS(5);

	RADIO_SDA = 1;
	
	delayUS(5);

	RADIO_SCL = 0;

}

void iicInit()
{
	RADIO_SCL = 0;

	iicStop();
}

void iicAck()
{
	RADIO_SDA = 0;

	RADIO_SCL = 1;

	delayUS(5);

	RADIO_SCL = 0;
}

/*  
	1 : have ack

*/
char iicCheckAck()
{
	uchar i = 0;
	
	RADIO_SDA = 1;

	RADIO_SCL = 1;

	delayUS(5);

	while( (1 == RADIO_SDA) && i < 255)
		i++;

	if(i == 255)
		return 0;
	else
		return 1;
}

void iicWriteByte(char wr)
{
	char i = 0;
	char temp = wr;
	
	for(i = 0; i < 8; i++)
	{
		temp = temp << 1;
		
		RADIO_SCL = 0;

		delayUS(5);
		
		RADIO_SDA = CY;

		delayUS(5);

		RADIO_SCL = 1;

		delayUS(5);
	}

	RADIO_SCL = 0;

	delayUS(5);

	RADIO_SDA = 1;

	delayUS(5);
	
}

char iicReadByte()
{
	char i = 0;
	char temp = 0;

	RADIO_SCL = 0;

	delayUS(5);

	RADIO_SDA = 1;

	for(i = 0; i < 8; i++)
	{
		
		RADIO_SCL = 1;

		delayUS(5);
		
		temp |= RADIO_SDA;

		RADIO_SCL = 0;

		delayUS(5);

		temp = temp << 1;
	}

	RADIO_SCL = 0;

	delayUS(5);

	RADIO_SDA = 1;

	delayUS(5);
	
	return temp;
}


