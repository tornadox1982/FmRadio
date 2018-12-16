/*
 * 七段数码管显示模块头文件
 * 
 */

#ifndef _LED_H_
#define _LED_H_

#include	"types.h"
#include	"config.h"



void showChar(char numb);
void showInt(int numb);
void showFloat(float fl);
void showHex(uint num);

void ledShowFloat(float fl) reentrant;

void ledShowSymbol(enum SYMBOL sym);


#endif
