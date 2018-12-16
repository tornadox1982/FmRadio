

#ifndef _TYPES_H
#define _TYPES_H


typedef unsigned char uchar;
typedef unsigned int  uint;
typedef unsigned short ushort;
typedef unsigned long  ulong;



#define TRUE		1
#define FALSE	0


union IRData
{
	uchar IRChar[4];
	ulong  IRLong;
};



typedef struct ChannelList 
{
	ushort validFlag;		/* 0xA55A is valid */
	ushort channels[31];		/* Store 31 channels in PLL */
} ChannelList;


#endif


