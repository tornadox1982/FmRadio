#include "types.h"
#include "config.h"

#include "intrins.h"


#define SECTOR_SIZE		512
#define TOTAL_SIZE		1024

enum {
	CHANNEL_SECTOR = 0,
	FAVORATE_SECTOR = 1
};


#if 0

sfr IAP_DATA	= 0xC2;
sfr IAP_ADDRH	= 0xC3;
sfr IAP_ADDRL	= 0xC4;
sfr IAP_CMD		= 0xC5;
sfr IAP_TRIG	= 0xC6;
sfr IAP_CONTR	= 0xC7;

#endif

#define CMD_IDLE	0
#define CMD_READ	1
#define CMD_PROG	2
#define CMD_ERASE	3

#define ENABLE_IAP	0x80	//sysclk < 30MHz

//#define IAP_ADDRESS	0x0000

static short channelStartAddr;
static short favoriteStartAddr;



void IapIdle()
{
	IAP_CONTR = 0;
	IAP_CMD = 0;
	IAP_TRIG = 0;
	IAP_ADDRL = 0;
	IAP_ADDRH = 0x80;	//Data ptr point to non-EEPROM area
}

char IapReadByte(int addr)
{
	char dat;

	IAP_CONTR = ENABLE_IAP;
	IAP_CMD = CMD_READ;

	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;

	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;

	_nop_();

	dat = IAP_DATA;

	IapIdle();

	return dat;
}

void IapWriteByte(int addr, char dat)
{
	IAP_CONTR = ENABLE_IAP;
	IAP_CMD = CMD_PROG;

	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;

	IAP_DATA = dat;

	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;

	_nop_();

	IapIdle();

	
}

void IapEraseSector(int addr)
{
	IAP_CONTR = ENABLE_IAP;
	IAP_CMD = CMD_ERASE;

	IAP_ADDRL = addr;
	IAP_ADDRH = addr >> 8;

	IAP_TRIG = 0x5A;
	IAP_TRIG = 0xA5;

	_nop_();

	IapIdle();

}

char loadLastValidChannelList(ChannelList *chl)
{
	ushort addr = CHANNEL_SECTOR * SECTOR_SIZE;
	char *ptr = (char *)chl;
	char i = 0;
	
	while(addr < SECTOR_SIZE)
	{
		*ptr++ = IapReadByte(addr);
		addr++;

		*ptr++ =  IapReadByte(addr);
		addr++;


		if(chl->validFlag == CHANNEL_VALID_FLAG)
		{
			//Set global addr
			channelStartAddr = addr - 2;

			while(addr < (channelStartAddr + sizeof(ChannelList)))
			{
				*ptr++ = IapReadByte(addr++);
			}

			//We have read a block, so we don't need to read another 
			break;
			
		}
		else
		{
			//To next channelList block
			addr += sizeof(ChannelList) -2;
		}
	}

	return TRUE;
}

/*
	Mode: 0: find a new block to store all Channels
		   1: store 
 */
char storeChannelList(ChannelList *chl)
{
	char* ptr = chl;
	
	ushort addr = channelStartAddr;

	if(chl == 0)
		return 0;

	/* First set current Channel List block as invalid */
	IapWriteByte(channelStartAddr, 0);
	IapWriteByte(channelStartAddr + 1, 0);

	//Change start address to a new block	
	channelStartAddr += sizeof(ChannelList);

	//Full of sector
	if(channelStartAddr == SECTOR_SIZE)
	{
		IapEraseSector(0);
		channelStartAddr = 0;		
	}

	for(addr = channelStartAddr; addr < channelStartAddr + sizeof(ChannelList); addr++)
	{
		IapWriteByte(addr, *ptr++);
	}
	
}

char storeSingleChannel(ChannelList *chl, char* index, unsigned short ch)
{
	char tmpIndex = *index;
	ushort addr;
	char* ptr = (char *)&ch;
	
	while(tmpIndex < 31)
	{
		//Find a not used channel
		if(chl->channels[tmpIndex] == 0xFFFF)
		{
			addr = channelStartAddr + &(chl->channels[tmpIndex]) - chl;

			IapWriteByte(addr, *ptr);
			addr++;
			ptr++;
			IapWriteByte(addr, *ptr);

			break;
		}

		tmpIndex++;
	}

	if(tmpIndex < 31)
	{
		//Change to new index
		*index = tmpIndex;
		return TRUE;
	}

	return FALSE;

}

char deleteSingleChannel(ChannelList *chl, char* index)
{
	ushort addr;

	chl->channels[*index] = 0;
	
	addr = channelStartAddr + &(chl->channels[*index]) - chl;

	IapWriteByte(addr, 0);
	addr++;
	IapWriteByte(addr, 0);

	(*index)++;

	return TRUE;


}

char getNextChannelIndex(ChannelList *chl, char index, char direct)
{
	char tmp;

	if(direct)
	{
		tmp = index + 1;

		if(tmp == 31)
		{
			tmp = 0;
		}
		
		while(tmp < 31)
		{
			//This channel has deleted
			if(chl->channels[tmp] == 0x0)
			{
				tmp++;
				continue;
			}
			else if(chl->channels[tmp] == 0xFFFF)
			{
				tmp = 0;
				break;
			}
			else
				break;
		}
	}
	else
	{
		tmp = index -1;

		/* It means that we should find a valid channel from the end of channel list */
		if(tmp < 0)
		{
			tmp = 30;
		}
		
		while(tmp >= 0)
		{
			/* The unused channel always is 0xFFFF */
			if(chl->channels[tmp] == 0xFFFF || chl->channels[tmp] == 0x0)
			{
				tmp--;
			}
			else
				break;
		}
					
	}

	/* Search out of boundary */
	if((direct && tmp == 31) || (!direct && tmp < 0))
		return 0;
	
	return tmp;
}

char storeFavoriteChannel(ushort ch)
{
	ushort pll = 0;
	char* temp = (char *)&pll;
	
	ushort addr = favoriteStartAddr;

	*temp = IapReadByte(addr);		
	addr++;
	
	*(temp+1) = IapReadByte(addr);
	addr++;

	/* current frequency equal to channel, no need to store */
	if(pll == ch)
		return FALSE;

	/* sector is full, first erase the sector and set address to the start of sector */
	if(addr == TOTAL_SIZE)
	{
		addr = FAVORATE_SECTOR * SECTOR_SIZE;
		IapEraseSector(addr);
	}

	temp = (char *)&ch;

	IapWriteByte(addr, *temp);
	IapWriteByte(addr + 1, *(temp + 1));

	favoriteStartAddr = addr;

	return TRUE;
		
}

unsigned short loadFavoriteChannel()
{
	ushort pll = 0;
	char* temp = (char *)&pll;
	ushort lastPll = 0;
	
	ushort addr = FAVORATE_SECTOR * SECTOR_SIZE;

	while(addr < TOTAL_SIZE)
	{
		*temp = IapReadByte(addr);		
		addr++;
		
		*(temp+1) = IapReadByte(addr);
		addr++;

		/* The current pll is   */		
		if(pll != 0xFFFF)
		{
			lastPll = pll;
		}
		else
		{
			/* get to the end of favorite partition */
			break;
		}
	}

	/* Store favoriteStartAddr as current frequency address */
	favoriteStartAddr = addr - 2;

	
	/* 0xFFFF is not a valid frequency, it's a end symbol of frequency, if the favorite channel partition is original,
	 *  return 0 is reasonable.
	*/
	return lastPll;
	
}

