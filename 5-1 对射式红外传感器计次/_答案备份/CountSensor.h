#ifndef __COUNT_SENSOR_H
#define __COUNT_SENSOR_H

#include <stdint.h>

/* 硬件消抖（可选，效果优于软件消抖，建议与软件消抖同时使用）：
 * 1. 传感器 DO 与 GND 之间并 0.1uF 电容，配合上拉电阻构成 RC 低通（时间常数约 1ms）；
 * 2. 信号线加施密特触发器整形（如 74HC14），或选用带迟滞比较器输出的模块；
 * 3. 模块供电就近加 0.1uF 去耦电容，信号线远离电机、继电器等干扰源。
 */

void CountSensor_Init(void);
void CountSensor_Update(void);		/* 需在主循环中反复调用，完成消抖确认与计数 */
uint32_t CountSensor_Get(void);
void CountSensor_Reset(void);

#endif
