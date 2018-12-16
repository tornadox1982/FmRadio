
#include "types.h"
#include "config.h"


/* ÿ�β�����Ҫ��RADIO_SCL��Ϊ�͵�ƽ, RADIO_SCL�ڸߵ�ƽ��ʱ���������λ�б仯����
	Ԥʾ���ǿ���λ������λֻ����RADIO_SCL�͵�ƽʱ���ܸı�
*/


/* ��ʼ����  RADIO_SCL ���Ǹߵ�ƽʱ,RADIO_SDA�ߴӸߵ�ƽ��͵�ƽ�л�*/
void iicStart()
{
	/* IIC���߿���ʱ���Ǹߵ�ƽ */
	RADIO_SDA = 1;
	RADIO_SCL = 1;

	delayUS(5);

	RADIO_SDA = 0;

	delayUS(5);

	RADIO_SCL = 0;
}

/* ��ֹ����  RADIO_SCL ���Ǹߵ�ƽʱ,RADIO_SDA�ߴӵ͵�ƽ��ߵ�ƽ�л�*/
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


