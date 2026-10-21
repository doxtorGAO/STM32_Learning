#include "stm32f10x.h"

/* ============================================================
 * Timer.c —— TIM2 定时中断，给整个工程提供 1ms 的"心跳"
 *
 * 思路：
 *   TIM2 挂在 APB1 上，时钟 72MHz。
 *   预分频 72 → 计数频率 1MHz（每 1 个数 = 1us）
 *   自动重装 1000 → 每 1000 个数 = 1000us = 1ms 产生一次更新中断
 *   中断里只做一件事：Timer_Tick++，它就是一个"开机以来过了多少毫秒"的秒表。
 *
 * 主循环随时可以调 Timer_GetTick() 读当前值，
 * 用两次读数的差就能算出中间过了多久 —— 这叫"时间戳差分法"，
 * 比 Delay_ms() 累加准得多，因为主循环哪怕被刷屏拖慢了，差值还是对的。
 * ============================================================ */

/* 中断里会改、主循环会读，必须加 volatile，
   否则编译器可能把它缓存进寄存器，主循环永远读到旧值 */
static volatile uint32_t Timer_Tick = 0;

/**
  * @brief  定时器初始化，配置 TIM2 每 1ms 中断一次
  * @param  无
  * @retval 无
  */
void Timer_Init(void)
{
	/* 1. 开 TIM2 的时钟（TIM2 在 APB1 总线上） */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

	/* 2. 选内部时钟源，也就是让 TIM2 用 72MHz 的 APB1 时钟计数 */
	TIM_InternalClockConfig(TIM2);

	/* 3. 时基单元：分频 + 计数模式 + 重装值 */
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;		/* 时钟分频，只影响滤波采样，填 DIV1 */
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;	/* 向上计数：0 数到重装值再归零 */
	TIM_TimeBaseInitStructure.TIM_Period = 1000 - 1;				/* ARR 自动重装值：数到 999 就溢出 */
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72 - 1;				/* PSC 预分频：72MHz / 72 = 1MHz */
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;			/* 高级定时器才用，TIM2 填 0 */
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

	/* 4. 先清一次更新标志，否则一开中断就会立刻进一次（这是个小坑） */
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);

	/* 5. 允许 TIM2 的更新中断（计数溢出时产生中断） */
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	/* 6. NVIC 里给 TIM2 中断通道放行 */
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;					/* TIM2 的中断通道 */
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);

	/* 7. 最后才启动定时器，前面配置的时候不要让它跑 */
	TIM_Cmd(TIM2, ENABLE);
}

/**
  * @brief  读"开机以来过了多少毫秒"
  * @param  无
  * @retval 毫秒数，从 0 开始，约 49.7 天回绕一次
  */
uint32_t Timer_GetTick(void)
{
	return Timer_Tick;
}

/**
  * @brief  TIM2 的中断服务函数
  * @note   函数名是启动文件里登记死的，一个字都不能改
  */
void TIM2_IRQHandler(void)
{
	/* 确认真的是"更新中断"进来了（中断里可能还有别的标志） */
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		Timer_Tick ++;								/* 心跳 +1 毫秒 */

		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);	/* 清标志，否则会一直重复进中断 */
	}
}
