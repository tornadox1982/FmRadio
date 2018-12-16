
#include "types.h"

/*
 * I2C���߳�ʼ����ʱ�Ӻ����ݶ�Ϊ�ߵ�ƽ
 */
void i2c_init(void);

/*
 * I2C������������ʱ��Ϊ�ߵ�ƽ�������ɸߵ�ƽ��͵�ƽ����Ϊ����
 */
void i2c_start(void);

/*
 * I2C����ֹͣ����ʱ��Ϊ�ߵ�ƽ�������ɵ͵�ƽ��ߵ�ƽ����Ϊ����
 */
void i2c_stop(void);

void i2c_ack(void);

void i2c_nack(void);

/*
 * ���I2C�ӻ��Ƿ���Ӧ�������Ӧ�𵫳�ʱҲ��
 */
static void checkack(void);

/*
 * I2C������дһ���ֽڵ�����
 */
void i2c_write_byte(uchar wbyte);

/*
 * I2C�����϶�һ���ֽڵ�����
 */
uchar i2c_read_byte(void);

void iicInit();

void iicStart();
void iicStop();

void iicAck();
char iicCheckAck();
void iicWriteByte(char wr);
char iicReadByte();
