#include "stm32f10x.h"
#include "Timer.h"

/* ============================================================
 * 定时器定时中断 —— 参考答案
 *
 * 时钟链路：
 *   72 MHz → ÷(7200-1+1)=7200 → 10 kHz → 数(10000-1+1)=10000 个 → 1 Hz
 *
 * 为什么 TIM2 是 72MHz 而不是 36MHz：
 *   TIM2 挂在 APB1 上，APB1 的分频系数是 2（36MHz），
 *   但 STM32 规定：APB 分频系数不为 1 时，定时器时钟 = PCLK × 2。
 *   所以 36 × 2 = 72MHz。这条规则很容易记错。
 * ============================================================ */

volatile int16_t Num;


void Timer_Init(void)
{
	/* TIM2 在 APB1 上，不是 APB2 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* 选内部时钟（默认值，写出来更清楚） */
	TIM_InternalClockConfig(TIM2);

	/* 时基单元 */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;		/* ARR */
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;		/* PSC */
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

	/* 清掉初始化时产生的更新标志，否则一开中断就立刻进一次 */
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	/* 开更新中断 */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	/* 启动定时器 —— 必须放在中断函数写好之后 */
	TIM_Cmd(TIM2, ENABLE);
}


void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		Num++;
	}
}
