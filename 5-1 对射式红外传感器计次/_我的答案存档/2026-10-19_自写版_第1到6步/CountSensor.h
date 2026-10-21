#ifndef __COUNT_SENSOR_H
#define __COUNT_SENSOR_H

#include <stdint.h>

/* ============================================================
 * 对射式红外传感器计次 —— 填空练习版（头文件）
 *
 * 头文件的作用：把"这个模块对外提供哪些函数"写清楚，给 main.c 看。
 * 函数名、参数、返回值必须和 .c 里的定义完全一致。
 *
 * 第 1 步：把这个文件写完（就是下面这几个声明），编译一次确认没问题。
 * ============================================================ */

/* 第 2~5 步：初始化 PB14、EXTI、NVIC */
void CountSensor_Init(void);

void CountSensor_Update(void);

/* 读取当前计数值 */
uint32_t CountSensor_Get(void);

/* 把计数清零 */
void CountSensor_Reset(void);

#endif


