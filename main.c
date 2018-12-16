
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

//是否有红外遥控信号
volatile uchar IRFlag = 0;

//红外线接收到的编码值
volatile union IRData IRCOM;

//当前收听的电台频率
volatile float currentFreq = 87.5;

//当前收听电台的PLL值
volatile ushort currentPLL;

//当前实时温度
volatile float currentTemperature = 0.0;

//当前工作模式 0：收音模式 1：温度显示模式
uchar xdata globalMode = TEMPERATURE_MODE;

//系统启动后经过的分钟数
//ulong xdata globalMinutes = 0;

//系统启动后经过的秒数
ulong xdata globalSeconds = 0;

//当前电台已经收听的分钟数
uint xdata currentStationTimes = 0;

//当前电台列表
ChannelList xdata globalChannelList;

//当前波段选择欧洲或者日本0: Europen 1: Japan
char xdata currentBandSwitch;

//当前收听电台在电台列表中的索引值
char xdata currentChannelIndex;


/******************** Function *******************************************/

/*
 *延时t微秒
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
 *延时t毫秒
 */
void delayMS(ushort ms)
{
	uchar i;
	while(ms--)
	{
		//时钟为24MHZ时，约延时1ms
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
	/* 外部中断0下降沿触发 */
	IT0 = 1;
	
	/* 使能外部中断0 */	
	EX0 = 1;
}

void IRInit()
{
	/* 外部中断1下降沿触发 */
	IT1 = 1;
	
	/* 使能外部中断1 */
	EX1 = 1;

	IR = 1;

	IRFlag = 0;
}

/*
 *	STC单片机串口初始化
 */
void serialInit(void)
{
	//不允许串口中断，只发送
	ES = 0;

	//工作于10位异步收发器(8位数据)，不允许接收
	SCON = 0x40;
		
	//使用独立波特率发生器作为串口波特率，并允许其运行
	AUXR  = 0x11;

	//bps: 2400 @24MHz osc
	//Compute method: 256 - 取整[晶振/波特率/32/12 + 0.5]
	//此时误差0.1%
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
 * 以base进制打印num
 * base可选2, 8, 10, 16
 * 若为八进制，则以0开头
 * 若为十六进制，则以0x开头
 *
 */
void putNum(int num, uchar base)
{
	uchar temp[6] = {'0'};

	char i = 0;

	/* 若为十进制且为负 */
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
		num = (uint)num / base;		//此处加强转是为了当值为负数时且为非十进制打印时能正确输出
		i++;
	}

	i--;

	while(i > 0)
	{
		putChar(temp[i--]);
	}

	/* 打印temp[0] */
	putChar(temp[i]);
	
}

void timerInit(void)
{
	TH0 = TIMER_COUNT_HIGH;  
	TL0 = TIMER_COUNT_LOW; 

	/* 方式1，内部启动，16位定时器，需要每次将定时器的值装入 */
	TMOD = 0x01;

	//使能T0中断
	ET0 = 1;

	//启动T0
	TR0 = 1;
	
}


/*
 *利用时间中断显示温度，T0 
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
 *	红外线中断程序，在中断程序中输出接收到的
 *	按键信息（单片机为大端模式，所以接收到按键
 *	与定义的一样）
 */
void  IRInt() interrupt 2
{ 
	unsigned char j,k,N=0; 

	IRFlag = 0;

	//首先关外部中断
	EX1 = 0;  

	//延时2ms,若还为高电平则不是红外信号	
	IRdelay(15);

	if (IR == 1) 
	{
		EX1 =1; 
		return; 
	} 

	//确认IR信号出现 
	while (!IR)            //等IR变为高电平，跳过9ms的前导低电平信号。 
	{
		IRdelay(1);
	}
	
	for (j=0;j<4;j++)         //收集四组数据 
	{ 
		for (k=0;k<8;k++)        //每组数据有8位 
		{ 
			while (IR)            //等 IR 变为低电平，跳过4.5ms的前导高电平信号。 
			{
				IRdelay(1);
			} 
			
			while (!IR)          //等 IR 变为高电平 
      			{
      				IRdelay(1);
			} 
			
			while (IR)           //计算IR高电平时长 
			{ 
				IRdelay(1); 
				N++;
				
				if (N>=30) 
				{
					EX1=1; 
					return;
				}                  //0.14ms计数过长自动离开。 
			} //高电平计数完毕   
			
			IRCOM.IRChar[j] = IRCOM.IRChar[j] >> 1;                  //数据最高位补“0” 
			
     		if (N>=8) 
			{
				IRCOM.IRChar[j] = IRCOM.IRChar[j] | 0x80;
			}  //数据最高位补“1” 
			
			N=0; 
		} 
 	} 

   	//判断接收的第三组数据和四组数据是否相反 ，
   	//若相反则证明接收到了正确的按键信息
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

	//按键按下时的时钟
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

	//清除IR
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

	//关闭中断
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
	
	//开启所有中断
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

