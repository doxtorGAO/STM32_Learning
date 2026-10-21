#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "CountSensor.h"

/* ======================= 传感器极性配置 =======================
 * 对射式红外模块：光束未被遮挡时 DO 输出高电平，被遮挡时 DO 输出低电平。
 * 【原代码的 bug】触发边沿写的是 EXTI_Trigger_Rising（上升沿），
 * 但中断里又要求引脚为低电平(==0)才计数。上升沿刚过时引脚是高电平，
 * 两个条件互相矛盾，因此计数基本不会增加（只有抖动噪声偶发命中）。
 * 修正原则：触发边沿与电平确认必须一致。
 * 若你的模块极性相反（遮挡时输出高电平），把下面两行同时改掉即可：
 *     #define COUNT_SENSOR_TRIGGER_EDGE   EXTI_Trigger_Rising
 *     #define COUNT_SENSOR_ACTIVE_LEVEL   1
 */
#define COUNT_SENSOR_TRIGGER_EDGE	EXTI_Trigger_Falling
#define COUNT_SENSOR_ACTIVE_LEVEL	0

/* 消抖时间(ms)：等电平稳定后再确认，滤掉机械抖动与电气毛刺 */
#define COUNT_SENSOR_DEBOUNCE_MS	10

/* ======================= 为什么中断里不延时 =======================
 * Delay_ms() 是独占 SysTick 忙等的（改写 LOAD/VAL/CTRL 并在结束时关闭定时器），
 * 而主循环里的 Key_GetNum() 也在调用 Delay_ms()。
 * 若在 EXTI 中断里调用 Delay_ms()，中断会改掉主循环那次延时的 SysTick 配置
 * 并关闭定时器，导致主循环的 while(!(SysTick->CTRL & 0x00010000)) 永远等不到
 * 计数标志而卡死。所以：中断只置标志，消抖确认放到主循环的 CountSensor_Update() 里。
 */
volatile uint8_t CountSensor_Flag;		/* 中断置位：检测到一次有效边沿，待主循环确认 */
uint32_t CountSensor_Count;				/* 计数值，uint32_t 不会在 65535 处回绕 */

void CountSensor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);

	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = COUNT_SENSOR_TRIGGER_EDGE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	CountSensor_Flag = 0;
	CountSensor_Count = 0;
}

/**
  * @brief  在主循环中反复调用：完成消抖确认与计数
  * @note   中断里只置 CountSensor_Flag，这里延时 10ms 等电平稳定后再确认电平，
  *         确认有效才计数，并在计数后静默一小段，避免同一段遮挡被重复计数。
  */
void CountSensor_Update(void)
{
	if (CountSensor_Flag)
	{
		Delay_ms(COUNT_SENSOR_DEBOUNCE_MS);		/* 消抖：等电平稳定 */
		CountSensor_Flag = 0;					/* 消抖期间抖动产生的边沿一并丢弃 */

		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == COUNT_SENSOR_ACTIVE_LEVEL)
		{
			CountSensor_Count ++;				/* 电平已确认有效，计一次 */
			Delay_ms(COUNT_SENSOR_DEBOUNCE_MS);	/* 静默期：屏蔽同一段遮挡的重复触发 */
			CountSensor_Flag = 0;
		}
	}
}

uint32_t CountSensor_Get(void)
{
	return CountSensor_Count;
}

void CountSensor_Reset(void)
{
	CountSensor_Count = 0;
}

void EXTI15_10_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line14) == SET)
	{
		EXTI_ClearITPendingBit(EXTI_Line14);	/* 先清挂起标志，避免重复进入 */
		CountSensor_Flag = 1;					/* 只记录事件，实际计数交给主循环消抖确认 */
	}
}
