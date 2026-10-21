#include "stm32f10x.h"
#include "Encoder.h"

/* ============================================================
 * 旋转编码器计次 —— 参考答案
 *
 * 硬件：EC11 旋转编码器模块
 *   A 相 → PB0
 *   B 相 → PB1
 *   模块板上一般自带 10k 上拉 + RC 滤波，下面的 IPU 算双保险。
 *
 * 转向判断原理：
 *   转动时 A、B 输出两路相位差 90° 的方波。
 *   正转：A 先变低，B 后变低 → A 的下降沿时 B 还是高；B 的下降沿时 A 已经低
 *   反转：B 先变低，A 后变低 → A 的下降沿时 B 已经低
 *   所以每个下降沿里读"另一相"的电平，就能判断方向。
 *   两相各管一个方向，一次转动只被数一次。
 * ============================================================ */

#define ENCODER_GPIO		GPIOB
#define ENCODER_PIN_A		GPIO_Pin_0
#define ENCODER_PIN_B		GPIO_Pin_1
#define ENCODER_PIN_A_SRC	GPIO_PinSource0
#define ENCODER_PIN_B_SRC	GPIO_PinSource1
#define ENCODER_LINE_A		EXTI_Line0
#define ENCODER_LINE_B		EXTI_Line1

volatile int16_t Encoder_Count;


void Encoder_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

	/* 两个引脚一次配完：GPIO_Pin 是位掩码，用 | 拼起来即可 */
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = ENCODER_PIN_A | ENCODER_PIN_B;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ENCODER_GPIO, &GPIO_InitStructure);

	/* 两条线各映射一次，不能省 */
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, ENCODER_PIN_A_SRC);
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, ENCODER_PIN_B_SRC);

	/* 两条线也一次配完 */
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = ENCODER_LINE_A | ENCODER_LINE_B;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* ---- 0 号和 1 号是两条独立的中断通道，要开两次 ---- */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_Init(&NVIC_InitStructure);

	Encoder_Count = 0;
}


/* 读一次就清零：返回的是"自上次调用以来转了几格" */
int16_t Encoder_Get(void)
{
	int16_t Temp;
	Temp = Encoder_Count;
	Encoder_Count = 0;
	return Temp;
}


/* A 相下降沿：若此时 B 已经为低，说明 B 领先 → 反转 */
void EXTI0_IRQHandler(void)
{
	if (EXTI_GetITStatus(ENCODER_LINE_A) == SET)
	{
		EXTI_ClearITPendingBit(ENCODER_LINE_A);		/* 先清位，后面怎么 return 都安全 */

		if (GPIO_ReadInputDataBit(ENCODER_GPIO, ENCODER_PIN_A) == 0)		/* 确认 A 真为低 */
		{
			if (GPIO_ReadInputDataBit(ENCODER_GPIO, ENCODER_PIN_B) == 0)	/* B 也是低 */
			{
				Encoder_Count--;
			}
		}
	}
}


/* B 相下降沿：若此时 A 已经为低，说明 A 领先 → 正转 */
void EXTI1_IRQHandler(void)
{
	if (EXTI_GetITStatus(ENCODER_LINE_B) == SET)
	{
		EXTI_ClearITPendingBit(ENCODER_LINE_B);

		if (GPIO_ReadInputDataBit(ENCODER_GPIO, ENCODER_PIN_B) == 0)
		{
			if (GPIO_ReadInputDataBit(ENCODER_GPIO, ENCODER_PIN_A) == 0)
			{
				Encoder_Count++;
			}
		}
	}
}
