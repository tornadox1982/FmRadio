/*
 * CCM162B2BҺ����ʾģ��ͷ�ļ�
 * ����1602Һ��
 */

#ifndef _DS18B20_H_
#define _DS18B20_H_

#include	"types.h"
#include	"config.h"

/* ROM �������� */
#define	ROM_READ_CMD	0x33	//����ROM���к�
#define	ROM_MATCH_CMD	0x55	//ƥ��ROM���к�
#define	ROM_SERCH_CMD	0xF0	//����ROM���к�
#define	ROM_SKIP_CMD		0xCC	//����ROM���к�
#define	ROM_ALARM_SERCH_CMD	0xEC	//�¶ȱ�����������


/* RAM �������� */
#define RAM_WRITE_SCRATCHPAD	0x4E	//д���ݵ������ݴ���TH��TL
#define RAM_READ_SCRATCHPAD	0xBE	//�Ӹ����ݴ���������
#define RAM_COPY_SCRATCHPAD	0x48	//���Ƹ����ݴ������ݵ�EEPROM

#define TEMP_CONVERT			0x44	//�¶�ת������
#define RECALL_EEPRAM			0xB8	//�ض�EEPRAM�������ݴ���
#define READ_POWER_SUPPLY	0xB4	//��ȡ��Դģʽ


void DS18B20Init();

void readRomSerialNumber(uchar* buf);

void convertTemperature();
float readTemperature();

#endif
