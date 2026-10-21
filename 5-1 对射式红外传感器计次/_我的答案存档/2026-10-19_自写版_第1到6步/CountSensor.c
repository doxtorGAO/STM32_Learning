#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "CountSensor.h"



#define COUNT_SENSOR_TRIGGER_EDGE	EXTI_Trigger_Falling
#define COUNT_SENSOR_ACTIVE_LEVEL	0

/* 消抖时间(ms) */
#define COUNT_SENSOR_DEBOUNCE_MS	10

/* 中断里写、主循环里读的变量，必须加 volatile，
   否则编译器可能以为它"没变过"而把判断优化掉。 */
volatile uint8_t CountSensor_Flag;
uint32_t CountSensor_Count;


void CountSensor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line=EXTI_Line14;
	EXTI_InitStructure.EXTI_LineCmd=ENABLE;
	EXTI_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger=EXTI_Trigger_Falling;
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


void EXTI15_10_IRQHandler(void)
{
	
	     if (EXTI_GetITStatus(EXTI_Line14) == SET)
	     {
	                     
	         EXTI_ClearITPendingBit(EXTI_Line14);
			 CountSensor_Flag =1;
	     }
	     
}


uint32_t CountSensor_Get(void)
{

	return CountSensor_Count;
}
