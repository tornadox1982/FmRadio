/*
 * CCM162B2B液晶显示模块头文件
 * 兼容1602液晶
 */

#ifndef _DS18B20_H_
#define _DS18B20_H_

#include	"types.h"
#include	"config.h"

/* ROM 操作命令 */
#define	ROM_READ_CMD	0x33	//读出ROM序列号
#define	ROM_MATCH_CMD	0x55	//匹配ROM序列号
#define	ROM_SERCH_CMD	0xF0	//搜索ROM序列号
#define	ROM_SKIP_CMD		0xCC	//跳过ROM序列号
#define	ROM_ALARM_SERCH_CMD	0xEC	//温度报警搜索命令


/* RAM 操作命令 */
#define RAM_WRITE_SCRATCHPAD	0x4E	//写数据到高速暂存器TH和TL
#define RAM_READ_SCRATCHPAD	0xBE	//从高速暂存器读数据
#define RAM_COPY_SCRATCHPAD	0x48	//复制高速暂存器数据到EEPROM

#define TEMP_CONVERT			0x44	//温度转换命令
#define RECALL_EEPRAM			0xB8	//重读EEPRAM到高速暂存器
#define READ_POWER_SUPPLY	0xB4	//读取电源模式


void DS18B20Init();

void readRomSerialNumber(uchar* buf);

void convertTemperature();
float readTemperature();

#endif
