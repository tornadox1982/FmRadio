/*
 *
 *
 *
 */

#include "iic.h"
#include "tea5767.h"

#include <string.h>

static	char rd_buffer[REGISTER_SIZE];
static	char wr_buffer[REGISTER_SIZE];

static  char validStation(char adc, char ifreq)
{
	if (adc > 3 && VALID_IF(ifreq))
		return TRUE;
	else
		return FALSE;
}

static void readRegister(char* buffer, char count)
{
	char i = 0;

	iicStart();  
	iicWriteByte(TEA5767_RD_BASE);        //TEA5767读地址  

	if(iicCheckAck())  
	{  
		for(i=0;i<count;i++)  
		{  
			buffer[i] = iicReadByte();  
			iicAck();  
		}  
	}

	iicStop();
}

static void writeRegister(char* buffer, char count)
{
	char i = 0;

	iicStart();  
	iicWriteByte(TEA5767_WR_BASE);
	if(iicCheckAck())  
	{  
		for(i=0;i<count;i++)  
		{  
			iicWriteByte(buffer[i]);  
			iicAck();  
		}  
	}
	
	iicStop();
}


void tea5767Init()
{
	
}


char sleepRadio()
{
	memset(wr_buffer, 0, REGISTER_SIZE);

	//mute
	wr_buffer[0] |= TEA5767_MUTE;

	//Standby and soft mute
	wr_buffer[3] |= TEA5767_SOFT_MUTE | TEA5767_STANDBY_MODE;

	writeRegister(wr_buffer, REGISTER_SIZE);

	return TRUE;
}

char wakupRadio(char bandSwitch, short pll)
{
	memset(wr_buffer, 0, REGISTER_SIZE);
	
	wr_buffer[0] = (pll >> 8) & 0x3F;
	wr_buffer[1] = pll & 0xFF;
	wr_buffer[2] = TEA5767_SEARCH_LOW_LEVEL;
	wr_buffer[3] = TEA5767_XTAL_32768K | TEA5767_SEARCH_INDEX;

	if(bandSwitch == JAPANESE_BAND)
		wr_buffer[3] |= TEA5767_JAPAN_BAND;
	
	wr_buffer[4] = 0;

	/* set freq */
	writeRegister(wr_buffer, REGISTER_SIZE);

	return TRUE;

}

char setFreq(char bandSwitch, short freq)
{
	//char adc, ifreq;
	
	short pll = 0;

	pll= LTOH_FREQ_TO_PLL(freq) ;
	
	memset(wr_buffer, 0, REGISTER_SIZE);

	wr_buffer[0] = (pll >> 8) & 0x3F;
	wr_buffer[1] = pll & 0xFF;
	wr_buffer[2] = TEA5767_SEARCH_MID_LEVEL;
	wr_buffer[3] = TEA5767_XTAL_32768K | TEA5767_SEARCH_INDEX;

	if(bandSwitch == JAPANESE_BAND)
		wr_buffer[3] |= TEA5767_JAPAN_BAND;
	
	wr_buffer[4] = 0;

	/* set freq */
	writeRegister(wr_buffer, REGISTER_SIZE);

	return TRUE;
	
}

char setPLL(char bandSwitch, short pll)
{
	//char adc, ifreq;
	
	memset(wr_buffer, 0, REGISTER_SIZE);

	wr_buffer[0] = (pll >> 8) & 0x3F;
	wr_buffer[1] = pll & 0xFF;
	wr_buffer[2] = TEA5767_SEARCH_MID_LEVEL;
	wr_buffer[3] = TEA5767_XTAL_32768K | TEA5767_SEARCH_INDEX;

	if(bandSwitch == JAPANESE_BAND)
		wr_buffer[3] |= TEA5767_JAPAN_BAND;
	
	wr_buffer[4] = 0;

	/* set freq */
	writeRegister(wr_buffer, REGISTER_SIZE);

	return TRUE;
	
}


/* 自动搜台并没有用自动搜台的模式，还是用的手动加判断输出 */
char autoScan(ChannelList* chl, char direct, char bandSwitch)
{
	char adc, ifreq;
	long pll;
	char i = 0;
	//ushort freq;

	/* Determize scan direct and starting frequency */	
	if(bandSwitch)
	{
		pll= direct ? LTOH_FREQ_TO_PLL(MIN_JAPAN_FREQ) : LTOH_FREQ_TO_PLL(MAX_JAPAN_FREQ);
	}
	else
	{
		pll= direct ? HTOL_FREQ_TO_PLL(MIN_EURO_FREQ) : HTOL_FREQ_TO_PLL(MAX_EURO_FREQ);
	}	

	/* Clear all register buffer */	
	memset(wr_buffer, 0, REGISTER_SIZE);
	memset(rd_buffer, 0 , REGISTER_SIZE);
	
	memset(chl, 0xFF, sizeof(ChannelList));	
	chl->validFlag = CHANNEL_VALID_FLAG;

	//Step mode 
	wr_buffer[0] = (pll >> 8)  | TEA5767_AUTO_SEARCH_MODE; 
	wr_buffer[1] = pll & 0xFF;
	
	wr_buffer[2] = TEA5767_SEARCH_LOW_LEVEL;

	if(direct)
	{
		wr_buffer[2] |= TEA5767_SEARCH_DIRECT_UP;
	}
	else
	{
		wr_buffer[2] |= TEA5767_HIGH_LO_INJECT_MODE;
	}
	
	if(bandSwitch == JAPANESE_BAND)
		wr_buffer[3] |= TEA5767_JAPAN_BAND;
	
	wr_buffer[3] = TEA5767_XTAL_32768K | TEA5767_SEARCH_INDEX;
	
	wr_buffer[4] = 0;

	/* set freq */
	writeRegister(wr_buffer, REGISTER_SIZE);

	do
	{

		//continue to read READY FLAG until it to 1
		do{
			delayMS(50);

			readRegister(rd_buffer, REGISTER_SIZE);
			
		} while (! (rd_buffer[0] & TEA5767_RADIO_READY_MASK));
#if 0
		/* Radio not ready, acrease or decrease pll and go on to scan */
		if(! (rd_buffer[0] & TEA5767_RADIO_READY_MASK))
		{
			if(direct)
				pll += STEP_FREQ;
			else
				pll -= STEP_FREQ;
			
			wr_buffer[0] = (pll >> 8);
			wr_buffer[1] = pll & 0xFF;

			continue;
		}
#endif
		ifreq = rd_buffer[2] & TEA5767_IF_CNTR_MASK;
		adc  = rd_buffer[3] >> 4;

		/* the station is clear, so return true */
		if(validStation(adc, ifreq))
		{
			pll = ((rd_buffer[0] & 0x3F) << 8) | rd_buffer[1];
			
			//Store the station to array
			if(i<30)
				chl->channels[i++] = pll;  
		}

		//for next scan
		if(direct)
			pll += PLL_STEP;
		else
			pll -= PLL_STEP;
		
		wr_buffer[0] = (pll >> 8);
		wr_buffer[1] = pll & 0xFF;

		/* only set freq */
		writeRegister(wr_buffer, 2);
	
	} while (rd_buffer[0] & TEA5767_BAND_LIMIT_MASK);

	/* At last set the freq to station 0 */
	setPLL(bandSwitch, chl->channels[0]);

	return TRUE;
	
}


