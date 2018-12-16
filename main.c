
#include "types.h"
#include "config.h"
#include "LED.h"
#include "DS18B20.h"
#include "TEA5767.h"
#include "eeprom.h"
#include "intrins.h"
#include "iic.h"

/*****************  Global Data  ************************************/

code uchar asciiTable[] = "0123456789ABCDEF";

//�Ƿ��к���ң���ź�
volatile uchar IRFlag = 0;

//�����߽��յ��ı���ֵ
volatile union IRData IRCOM;

//��ǰ�����ĵ�̨Ƶ��
volatile float currentFreq = 87.5;

//��ǰ������̨��PLLֵ
volatile ushort currentPLL;

//��ǰʵʱ�¶�
volatile float currentTemperature = 0.0;

//��ǰ����ģʽ 0������ģʽ 1���¶���ʾģʽ
uchar xdata globalMode = TEMPERATURE_MODE;

//ϵͳ�����󾭹��ķ�����
//ulong xdata globalMinutes = 0;

//ϵͳ�����󾭹�������
ulong xdata globalSeconds = 0;

//��ǰ��̨�Ѿ������ķ�����
uint xdata currentStationTimes = 0;

//��ǰ��̨�б�
ChannelList xdata globalChannelList;

//��ǰ����ѡ��ŷ�޻����ձ�0: Europen 1: Japan
char xdata currentBandSwitch;

//��ǰ������̨�ڵ�̨�б��е�����ֵ
char xdata currentChannelIndex;


/******************** Function *******************************************/

/*
 *��ʱt΢��
 */
void delayUS(ushort us)
{
	uchar i;
	while(us--)
	{
		_nop_();
		_nop_();
	
		i = 3;
		while (--i);
	}
}


/*
 *��ʱt����
 */
void delayMS(ushort ms)
{
	uchar i;
	while(ms--)
	{
		//ʱ��Ϊ24MHZʱ��Լ��ʱ1ms
		for(i=0; i< 250; i++) 
		{ ; }
	}
}

void IRdelay(uchar x)    //x*0.14MS 
{ 
	uchar i, j;
	//x = x;
	i = 4;
	j = 65;

	do
	{
		do
		{
			while (--j);
		} while (--i);
		
	}while(--x);
}

void extInit()
{
	/* �ⲿ�ж�0�½��ش��� */
	IT0 = 1;
	
	/* ʹ���ⲿ�ж�0 */	
	EX0 = 1;
}

void IRInit()
{
	/* �ⲿ�ж�1�½��ش��� */
	IT1 = 1;
	
	/* ʹ���ⲿ�ж�1 */
	EX1 = 1;

	IR = 1;

	IRFlag = 0;
}

/*
 *	STC��Ƭ�����ڳ�ʼ��
 */
void serialInit(void)
{
	//���������жϣ�ֻ����
	ES = 0;

	//������10λ�첽�շ���(8λ����)�����������
	SCON = 0x40;
		
	//ʹ�ö��������ʷ�������Ϊ���ڲ����ʣ�������������
	AUXR  = 0x11;

	//bps: 2400 @24MHz osc
	//Compute method: 256 - ȡ��[����/������/32/12 + 0.5]
	//��ʱ���0.1%
	BRT = 0xE6;
}


void putChar(uchar c)
{
	SBUF = c;

	while(TI == 0);

	TI = 0;
}

void putString(uchar *str)
{
	while(*str)
	{
		putChar(*str);
		str++;
	}
}


/*
 * ��base���ƴ�ӡnum
 * base��ѡ2, 8, 10, 16
 * ��Ϊ�˽��ƣ�����0��ͷ
 * ��Ϊʮ�����ƣ�����0x��ͷ
 *
 */
void putNum(int num, uchar base)
{
	uchar temp[6] = {'0'};

	char i = 0;

	/* ��Ϊʮ������Ϊ�� */
	if(num < 0 && base == 10)
	{
		putChar('-');
		num = -num;
	}

	if(base == 8)
	{
		putChar('0');
	}

	if(base == 16)
	{
		putChar('0'); putChar('x');
	}

	if(num ==0)
	{
		putChar('0');
		return;
	}

	while(num)
	{
		temp[i] = asciiTable[(uint)num % base];
		num = (uint)num / base;		//�˴���ǿת��Ϊ�˵�ֵΪ����ʱ��Ϊ��ʮ���ƴ�ӡʱ����ȷ���
		i++;
	}

	i--;

	while(i > 0)
	{
		putChar(temp[i--]);
	}

	/* ��ӡtemp[0] */
	putChar(temp[i]);
	
}

void timerInit(void)
{
	TH0 = TIMER_COUNT_HIGH;  
	TL0 = TIMER_COUNT_LOW; 

	/* ��ʽ1���ڲ�������16λ��ʱ������Ҫÿ�ν���ʱ����ֵװ�� */
	TMOD = 0x01;

	//ʹ��T0�ж�
	ET0 = 1;

	//����T0
	TR0 = 1;
	
}


/*
 *����ʱ���ж���ʾ�¶ȣ�T0 
 */
void timer0Int(void) interrupt 1
{
	static char count = 0;
	
	TH0 = TIMER_COUNT_HIGH;  
	TL0 = TIMER_COUNT_LOW; 

	++count;

	//one second past
	if(count == 32)
	{
		count = 0;

		globalSeconds++;		
	}
}


/*
 *	�������жϳ������жϳ�����������յ���
 *	������Ϣ����Ƭ��Ϊ���ģʽ�����Խ��յ�����
 *	�붨���һ����
 */
void  IRInt() interrupt 2
{ 
	unsigned char j,k,N=0; 

	IRFlag = 0;

	//���ȹ��ⲿ�ж�
	EX1 = 0;  

	//��ʱ2ms,����Ϊ�ߵ�ƽ���Ǻ����ź�	
	IRdelay(15);

	if (IR == 1) 
	{
		EX1 =1; 
		return; 
	} 

	//ȷ��IR�źų��� 
	while (!IR)            //��IR��Ϊ�ߵ�ƽ������9ms��ǰ���͵�ƽ�źš� 
	{
		IRdelay(1);
	}
	
	for (j=0;j<4;j++)         //�ռ��������� 
	{ 
		for (k=0;k<8;k++)        //ÿ��������8λ 
		{ 
			while (IR)            //�� IR ��Ϊ�͵�ƽ������4.5ms��ǰ���ߵ�ƽ�źš� 
			{
				IRdelay(1);
			} 
			
			while (!IR)          //�� IR ��Ϊ�ߵ�ƽ 
      			{
      				IRdelay(1);
			} 
			
			while (IR)           //����IR�ߵ�ƽʱ�� 
			{ 
				IRdelay(1); 
				N++;
				
				if (N>=30) 
				{
					EX1=1; 
					return;
				}                  //0.14ms���������Զ��뿪�� 
			} //�ߵ�ƽ�������   
			
			IRCOM.IRChar[j] = IRCOM.IRChar[j] >> 1;                  //�������λ����0�� 
			
     		if (N>=8) 
			{
				IRCOM.IRChar[j] = IRCOM.IRChar[j] | 0x80;
			}  //�������λ����1�� 
			
			N=0; 
		} 
 	} 

   	//�жϽ��յĵ��������ݺ����������Ƿ��෴ ��
   	//���෴��֤�����յ�����ȷ�İ�����Ϣ
	if (IRCOM.IRChar[2] != ~IRCOM.IRChar[3])     
	{
		EX1=1; 
		return;
	}
	
	IRFlag = 1;	
	EX1 = 1; 
} 

void extInt() interrupt 0
{
	//Delay for buttom press stable
	delayMS(2);

	if(globalMode == TEMPERATURE_MODE)
	{
		globalMode = RADIO_MODE;
	}
	else
	{
		globalMode = TEMPERATURE_MODE;
	}
}

/* 
 * The press process implied have priority.
 * From high priority to low is LEFT -> RIGHT -> UP -> DOWN
 */
void processButton(){

	//��������ʱ��ʱ��
	ulong lastSeconds;

	lastSeconds = globalSeconds;

		if(KEY_LEFT == 0)
		{
			delayMS(5);

			/* it's not a true press */
			if(KEY_LEFT == 1)
				return;
				
			/* wait for button bounced */
			while(KEY_LEFT == 0)
				;
			
			/* it's a long press, so we need to scan all stations from 108.0MHz  to 87.5MHz */
			if(globalSeconds - lastSeconds > 3)
			{
				autoScan(&globalChannelList, TEA5767_SEARCH_DOWN, currentBandSwitch);

				/* Alloc a new block to store all channels */
				storeChannelList(&globalChannelList);

				/* Start to listen channel 0 */
				currentChannelIndex = 0;
				currentPLL =  globalChannelList.channels[currentChannelIndex];
				setPLL(currentBandSwitch, currentPLL);

			}
			else
			{
				/* set frequency to the last valid station */
				//if(currentChannelIndex == 0)
					//currentChannelIndex = validStationsCount -1; 

				currentChannelIndex = getNextChannelIndex(&globalChannelList, currentChannelIndex, TEA5767_SEARCH_DOWN);
				currentPLL =  globalChannelList.channels[currentChannelIndex];
				setPLL(currentBandSwitch, currentPLL);
			}

			/* Translate to MHz */
			currentFreq = LTOH_PLL_TO_FREQ(globalChannelList.channels[currentChannelIndex]) / 100.0;
	
		}


		if(KEY_RIGHT == 0)
		{
			delayMS(5);

			/* it's not a true press */
			if(KEY_RIGHT == 1)
				return;
				
			/* wait for button bounced */
			while(KEY_RIGHT == 0)
				;
			
			/* it's a long press, so we need to scan all stations from 87.5MHz to 108.0MHz*/
			if(globalSeconds - lastSeconds > 3)
			{
				autoScan(&globalChannelList, TEA5767_SEARCH_UP, currentBandSwitch);
				storeChannelList(&globalChannelList);

				currentChannelIndex = 0;

				currentPLL =  globalChannelList.channels[currentChannelIndex];
				setPLL(currentBandSwitch, currentPLL);

			}
			else
			{
				/* set frequency to the last valid station */
				currentChannelIndex = getNextChannelIndex(&globalChannelList, currentChannelIndex, TEA5767_SEARCH_UP);
				
				setPLL(currentBandSwitch, globalChannelList.channels[currentChannelIndex]);
			}

			/* Translate to MHz */
			currentFreq = LTOH_PLL_TO_FREQ(globalChannelList.channels[currentChannelIndex]) / 100.0;
	
		}

		if(KEY_UP == 0)
		{
			delayMS(5);

			/* it's not a true press */
			if(KEY_UP == 1)
				return;
				
			/* wait for button bounced */
			while(KEY_UP == 0)
				;
			
			/* it's a long press, so we need to scan all stations from 87.5MHz to 108.0MHz*/
			if(globalSeconds - lastSeconds > 3)
			{
				storeSingleChannel(&globalChannelList, &currentChannelIndex, LTOH_FREQ_TO_PLL((ulong)(currentFreq * 1000)));
				currentPLL = globalChannelList.channels[currentChannelIndex];
				currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
				setPLL(currentBandSwitch, currentPLL);
			}
			else
			{			
				/* Add a step of 0.1MHz and set to listen to it */
				if(currentBandSwitch == EUROPEAN_BAND && currentFreq == MAX_EURO_FREQ_MHZ)
				{
					currentFreq = MIN_EURO_FREQ_MHZ;
				}
				else if (currentBandSwitch == JAPANESE_BAND && currentFreq == MAX_JAPAN_FREQ_MHZ)
				{
					currentFreq = MIN_JAPAN_FREQ_MHZ;
				}
				else
				{
					currentFreq += 0.1;
				}
				
				setFreq(currentBandSwitch, currentFreq);
			}	
		}

		if(KEY_DOWN == 0)
		{
			delayMS(5);

			/* it's not a true press */
			if(KEY_DOWN == 1)
				return;
				
			/* wait for button bounced */
			while(KEY_DOWN == 0)
				;
			
			/* it's a long press, so we need to delete */
			if(globalSeconds - lastSeconds > 3)
			{
				deleteSingleChannel(&globalChannelList, &currentChannelIndex);
				currentPLL = globalChannelList.channels[currentChannelIndex];
				currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
				setPLL(currentBandSwitch, currentPLL);				
			}
			else
			{
				/* Decrease a step of 0.1MHz and set to listen to it */
				if(currentBandSwitch == EUROPEAN_BAND && currentFreq == MIN_EURO_FREQ_MHZ)
				{
					currentFreq = MAX_EURO_FREQ_MHZ;
				}
				else if (currentBandSwitch == JAPANESE_BAND && currentFreq == MIN_JAPAN_FREQ_MHZ)
				{
					currentFreq = MAX_JAPAN_FREQ_MHZ;
				}
				else
				{
					currentFreq -= 0.1;
				}
				
				setFreq(currentBandSwitch, currentFreq);
			}

	
		}

}

void processIR()
{

#ifdef DEBUG
	putString("IR code: ");
	putNum(IRCOM.IRLong, 16);
	putChar('\n');
#endif

	switch(IRCOM.IRLong)
	{
		case IR_CHANNEL_0:
			currentChannelIndex = 0;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;

		case IR_CHANNEL_1:
			currentChannelIndex = 1;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_2:
			currentChannelIndex = 2;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_3:
			currentChannelIndex = 3;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_4:
			currentChannelIndex = 4;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_5:
			currentChannelIndex = 5;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_6:
			currentChannelIndex = 6;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_7:
			currentChannelIndex = 7;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_8:
			currentChannelIndex = 8;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_9:
			currentChannelIndex = 9;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_MODE_SWITCH:
			if(globalMode == TEMPERATURE_MODE)
			{
				globalMode = RADIO_MODE;
				wakupRadio(currentBandSwitch, currentPLL);
			}
			else
			{
				globalMode = TEMPERATURE_MODE;
				currentTemperature = readTemperature();
				sleepRadio();
			}
			break;

		case IR_STANDBY_RADIO:
			sleepRadio();			
			break;
			
		case IR_WAKEUP_RADIO:
			wakupRadio(currentBandSwitch, currentPLL);
			break;
			
		case IR_AUTO_UP_SCAN:
			autoScan(&globalChannelList, TEA5767_SEARCH_UP, currentBandSwitch);
			storeChannelList(&globalChannelList);
			currentChannelIndex = 0;
			currentPLL = globalChannelList.channels[currentChannelIndex];
			setPLL(currentBandSwitch, currentPLL);
			break;

		case IR_AUTO_DOWN_SCAN:
			autoScan(&globalChannelList, TEA5767_SEARCH_DOWN, currentBandSwitch);
			storeChannelList(&globalChannelList);
			currentChannelIndex = 0;
			currentPLL = globalChannelList.channels[currentChannelIndex];			
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_UP:
			currentChannelIndex = getNextChannelIndex(&globalChannelList, currentChannelIndex, TEA5767_SEARCH_UP);
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_CHANNEL_DOWN:
			currentChannelIndex = getNextChannelIndex(&globalChannelList, currentChannelIndex, TEA5767_SEARCH_DOWN);
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		case IR_FREQ_UP:
			currentFreq += 0.1;
			setFreq(currentBandSwitch, currentFreq);
			break;
			
		case IR_FREQ_DOWN:
			currentFreq -= 0.1;
			setFreq(currentBandSwitch, currentFreq);
			break;
			
		case IR_DELETE_CHANNEL:
			deleteSingleChannel(&globalChannelList, &currentChannelIndex);
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);				
			break;
			
		case IR_STORE_CHANNEL:
			storeSingleChannel(&globalChannelList, &currentChannelIndex, LTOH_FREQ_TO_PLL((ulong)(currentFreq * 1000)));
			currentPLL = globalChannelList.channels[currentChannelIndex];
			currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
			setPLL(currentBandSwitch, currentPLL);
			break;
			
		default:
			break;
	}

	//���IR
	IRFlag = 0;
}


void dumpChannelList(ChannelList* chl)
{
	char i = 0;
	
	if(chl == 0)
		return;

	for(i=0; i< 31; i++)
	{
		putString("Channel: ");
		putNum(i, 10);
		putString("  ");
		putNum(chl->channels[i], 16);
		putString("\n");
	}

}




int main()
{

#ifdef DEBUG
	char i = 0;
	uchar serialNumber[8];
#endif

	//�ر��ж�
	EA = 0;
	
	KEY_UP = 1;
	KEY_DOWN = 1;
	KEY_LEFT = 1;
	KEY_RIGHT = 1;
	BAND_SWITCH = 1;

	IRCOM.IRLong = 0;

	iicInit();
	extInit();
	IRInit();
	DS18B20Init();
	serialInit();
	timerInit();
	tea5767Init();

	if(BAND_SWITCH == 0)
		currentBandSwitch = JAPANESE_BAND;
	
	//���������ж�
	EA  = 1;	

#ifdef DEBUG
	readRomSerialNumber(serialNumber);
	putString("DS18B20 Serial Numer: ");
	for(i=0; i<8; i++)
	{
		putNum(serialNumber[i], 16);
		putChar(' ');
	}
	putChar('\n');	
#endif

	currentTemperature = readTemperature();	
	currentPLL = loadFavoriteChannel();

	/* Found favorite channel, first listen it */
	if(currentPLL != 0)
	{
		currentFreq = LTOH_PLL_TO_FREQ(currentPLL);
		setPLL(currentBandSwitch, currentPLL);
	}

	loadLastValidChannelList(&globalChannelList);

	/* main task */
	while(1)
	{


		//Get IR code
		if(IRFlag)
		{
			processIR();
		}
		
		if((P0 & 0xF0) != 0xF0)
		{
			processButton();
		}

#if 0
		if(globalMode == RADIO_MODE && globalMinutes % 10 == 0)
		{
			storeFavoriteChannel(currentPLL);

			#ifdef DEBUG
				dumpChannelList(&globalChannelList);	
			#endif
		}

#endif
		//work for temperature mode, convert temerature every minute
		if((globalSeconds % 30) == 0)
		{

#ifdef DEBUG
	putString("Current time: ");
	putNum(globalSeconds, 10);
	putChar('\n');
	dumpChannelList(&globalChannelList);
#endif				
			if(globalMode == TEMPERATURE_MODE)
			{
				currentTemperature = readTemperature();
				convertTemperature();
			}
			else if(globalMode == RADIO_MODE && globalSeconds % 600 == 0)
			{
			
				//store current channel as favorite every 10 minute
				storeFavoriteChannel(currentPLL);

			
			}
		}

		if(globalMode == TEMPERATURE_MODE)
		{
			ledShowFloat(currentTemperature);
		}
		else
		{
			ledShowFloat(currentFreq);
		}
		
	}/* end while */

}

