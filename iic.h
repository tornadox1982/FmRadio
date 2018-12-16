
#include "types.h"

/*
 * I2C总线初始化，时钟和数据都为高电平
 */
void i2c_init(void);

/*
 * I2C总线启动，当时钟为高电平，数据由高电平向低电平跳变为启动
 */
void i2c_start(void);

/*
 * I2C总线停止，当时钟为高电平，数据由低电平向高电平跳变为启动
 */
void i2c_stop(void);

void i2c_ack(void);

void i2c_nack(void);

/*
 * 检查I2C从机是否有应答，如果无应答但超时也算
 */
static void checkack(void);

/*
 * I2C总线上写一个字节的数据
 */
void i2c_write_byte(uchar wbyte);

/*
 * I2C总线上读一个字节的数据
 */
uchar i2c_read_byte(void);

void iicInit();

void iicStart();
void iicStop();

void iicAck();
char iicCheckAck();
void iicWriteByte(char wr);
char iicReadByte();
