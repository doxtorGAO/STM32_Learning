#include "stm32f10x.h"
#include "Timer.h"

/* ============================================================
 * 定时器定时中断 —— 填空练习版
 *
 * 这一课换思路：
 *   EXTI 是"外面有事件，我去响应"——得有信号进来。
 *   定时器是"时间到了，我自己动"——不需要任何外部信号。
 *
 * 目标：让 TIM2 每 1 秒产生一次更新中断，中断里 Num++
 *
 * 时间怎么算出来（先看懂这个，再去填第 3 步）：
 *
 *   TIM2 的时钟 = 72 MHz
 *   ↓ 除以 (PSC+1) = 7200
 *   计数频率   = 10 kHz        ← 每 100 微秒数一个数
 *   ↓ 数 (ARR+1) = 10000 个
 *   溢出频率   = 1 Hz          ← 正好 1 秒一次
 *
 *   所以：72 000 000 ÷ 7200 ÷ 10000 = 1
 *
 * 填空顺序（一次只填一块，编译一次，烧录一次）：
 *   第 2 步  main 里把 Num 显示出来（此时一直是 0，验证显示通路）
 *   第 3 步  打开 TIM2 时钟 + 选择内部时钟源
 *   第 4 步  配时基单元（PSC / ARR / 计数模式 …）
 *   第 5 步  清一次更新标志
 *   第 6 步  开更新中断 + 配 NVIC
 *   第 7 步  写 TIM2_IRQHandler，再启动定时器
 *
 * 参考答案在工程根目录的 _答案备份 文件夹里，卡住了再看。
 * ============================================================ */

volatile int16_t Num;


void Timer_Init(void)
{
	/* TODO 第 3 步（1）：打开 TIM2 的时钟
	   —— 注意！TIM2 挂在 **APB1** 上，不是 APB2。
	      函数是 RCC_APB1PeriphClockCmd，宏是 RCC_APB1Periph_TIM2。
	      写成 APB2 的话编译器会报"未定义的标识符"，
	      因为 APB2 那一组宏里根本没有 TIM2。 */

	/* TODO 第 3 步（2）：选择内部时钟源
	   —— TIM_InternalClockConfig(TIM2);
	      定时器有好几种时钟来源（内部 / 外部引脚 / 编码器），
	      内部时钟是默认值，但显式写一次能让读代码的人一眼看清。 */

	/* TODO 第 4 步：配置时基单元
	   —— 五个成员：
	      TIM_ClockDivision     = TIM_CKD_DIV1        （只影响输入滤波，随便填）
	      TIM_CounterMode       = TIM_CounterMode_Up  （向上计数）
	      TIM_Period            = 10000 - 1           （ARR，自动重装值）
	      TIM_Prescaler         = 7200 - 1            （PSC，预分频）
	      TIM_RepetitionCounter = 0                   （高级定时器才用，这里填 0）
	   —— 最后 TIM_TimeBaseInit(TIM2, &结构体);
	   —— 为什么要 -1：硬件从 0 开始数，数到 PSC 才归零，
	      所以实际分频是 (PSC+1)。ARR 同理，实际计数个数是 (ARR+1)。
	      填 7200 和 10000 的话，实际是 1.0001 秒，慢 0.01%。 */

	/* TODO 第 5 步：清一次更新标志
	   —— TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	   —— 这一步看着多余，其实很关键：
	      TIM_TimeBaseInit 写完的瞬间，硬件会把"更新事件"标志置 1
	      （相当于一上电就先溢出了一次）。
	      不先清掉的话，一开中断就立刻进一次 ISR，
	      现象是程序刚跑起来 Num 就已经是 1，不是从 0 开始。
	   —— 位置必须在 TimeBaseInit 之后、ITConfig 之前。 */

	/* TODO 第 6 步（1）：打开更新中断
	   —— TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); */

	/* TODO 第 6 步（2）：配 NVIC
	   —— 和之前两课一模一样：分组 + 结构体 + NVIC_Init。
	      中断通道是 TIM2_IRQn（注意不是 TIM2_UP_IRQn，
	      那是给"更新"以外的事件用的，这里用不到）。 */

	/* 第 7 步的 TIM_Cmd 放在这下面，等你写完中断函数再加 */
}


void TIM2_IRQHandler(void)
{
	/* TODO 第 7 步（1）：更新中断服务函数
	   思路和 EXTI 完全一样：
	     1) 判断 TIM_GetITStatus(TIM2, TIM_IT_Update) == SET；
	     2) 清挂起：TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	     3) Num++；
	   函数名必须叫 TIM2_IRQHandler，写在 Timer.c 里就行，
	   放哪个文件都可以，但名字不能改。 */

	/* TODO 第 7 步（2）：然后回到 Timer_Init 末尾加一句启动定时器
	   —— TIM_Cmd(TIM2, ENABLE);
	   —— 注意顺序：**先写完这个中断函数，再加 TIM_Cmd**。
	      如果反过来，中断已经开了、定时器也跑起来了，但中断函数还空着，
	      第一次更新事件会跳进启动文件里的默认死循环，程序直接卡死。 */
}
