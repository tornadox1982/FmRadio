
#include "DS18B20.h"
#include "intrins.h"

static void reset()
{
    CY = 1;
    while (CY)
    {
        DQ = 0;                     //送出低电平复位信号
        delayUS(480);              //延时至少480us
        DQ = 1;                     //释放数据线
        delayUS(60);               //等待60us
        CY = DQ;                    //检测存在脉冲
        delayUS(420);              //等待设备释放数据线
    }

}

/**************************************
从DS18B20读1字节数据
**************************************/
static uchar ReadByte()
{
    char i;
    uchar dat = 0;

    for (i=0; i<8; i++)             //8位计数器
    {
        dat >>= 1;
        DQ = 0;                     //开始时间片
        _nop_();                    //延时等待
        _nop_();
        DQ = 1;                     //准备接收
        _nop_();                    //接收延时
        _nop_();
        if (DQ) dat |= 0x80;        //读取数据
        delayUS(60);               //等待时间片结束
    }

    return dat;
}

/**************************************
向DS18B20写1字节数据
**************************************/
static void WriteByte(uchar dat)
{
    char i;

    for (i=0; i<8; i++)             //8位计数器
    {
        DQ = 0;                     //开始时间片
        _nop_();                    //延时等待
        _nop_();
        dat >>= 1;                  //送出数据
        DQ = CY;
        delayUS(60);               //等待时间片结束
        DQ = 1;                     //恢复数据线
    }
}

void readRomSerialNumber(uchar* buf)
{
	uchar i;
	
	if (buf == (uchar *)0)
		return;

	reset();

	WriteByte(ROM_READ_CMD);

	for(i=0; i<8; i++)
	{
		buf[i] = ReadByte();
	}
}

uchar getPowerMode()
{
	reset();
	
	WriteByte(READ_POWER_SUPPLY);

	if (DQ)
		return 1;
	else
		return 0;
}


void convertTemperature()
{	

	reset();

	WriteByte(ROM_SKIP_CMD);
	
	WriteByte(TEMP_CONVERT);	

	/* 如果处于温度转换中，则DQ一直为0，转换完成后为1 */
	while(!DQ);	
}

float readTemperature()
{
	char temp = 0;
	uchar lsb = 0;
	uchar msb = 0;
	float realTemperature = 0.0;

	reset();

	WriteByte(ROM_SKIP_CMD);
	
	WriteByte(RAM_READ_SCRATCHPAD);

	lsb = ReadByte();
	msb = ReadByte();

	temp = (msb << 4) |(lsb >> 4);
	//return temp;
	//ret = (msb << 8) | lsb;
	//ret = ret & 0xF;

	//first add
 	realTemperature += temp;

	if(lsb & 0x08)
		realTemperature += 0.5;
	
	if(lsb & 0x04)
		realTemperature += 0.25;
	
	if(lsb & 0x02)
		realTemperature += 0.125;
	
	if(lsb & 0x01)
		realTemperature += 0.0625;
	
	return realTemperature;
	

}

char setMode(char th, char tl, uchar mode)
{
	uchar precise;
	
	if (th > 125 || tl < -55)
		return -1;

	if (tl > th)
		return -1;
	
	if(mode < 9 || mode > 12)
		return -1;

	switch(mode)
	{
		case 9:
			precise = 0x1F;
			break;
		case 10:
			precise = 0x3F;
			break;
		case 11:
			precise = 0x5F;
			break;
		case 12:
			precise = 0x7F;
			break;
		default:
			precise = 0x7F;
	}

	reset();
	
	WriteByte(ROM_SKIP_CMD);	
	
	WriteByte(RAM_WRITE_SCRATCHPAD);
	
	WriteByte(th);	
	
	WriteByte(tl);
	
	WriteByte(precise);

	return 0;
	
}

void getMode(char* th, char* tl, uchar* mode)
{
	reset();
	
	WriteByte(ROM_SKIP_CMD);	
	
	WriteByte(RAM_READ_SCRATCHPAD);

	/* dummy read */
	ReadByte();	
	ReadByte();	
	
	*th = ReadByte();	
	
	*tl = ReadByte();	
	
	*mode = ReadByte();	
	
}
void saveMode()
{	
	reset();
	
	WriteByte(ROM_SKIP_CMD);	
	
	WriteByte(RAM_COPY_SCRATCHPAD);

	/* 在拷贝暂存器数据到EEPROM时要保持一个强上拉
	  * 时间至少10ms
	  */
	while(!DQ );	
}

char searchAlarm()
{

	return 0;	
}

void DS18B20Init()
{
	setMode(25, -30, 10);

	convertTemperature();
	
//	saveMode();
}

