#ifndef __KEY_H
#define __KEY_H

#include "stm32f10x.h"

/* Key_GetNum() 的返回值：一次读取只报一个"事件" */
#define KEY_NONE		0		/* 没有按键事件 */
#define KEY1_SHORT		1		/* KEY1 短按（松手那一刻） */
#define KEY2_SHORT		2		/* KEY2 短按（松手那一刻） */
#define KEY2_LONG		3		/* KEY2 长按满 1 秒（按住不放时立刻上报） */

void Key_Init(void);
uint8_t Key_GetNum(void);

#endif
