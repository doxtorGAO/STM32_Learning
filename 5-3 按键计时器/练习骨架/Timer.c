#include "stm32f10x.h"

/* ============================================================
 * Timer.c —— TIM2 定时中断（填空练习版）
 *
 * 这个模块只干一件事：让 Timer_Tick 每 1 毫秒自己 +1。
 * 它就像一块表，上电开始滴答，谁都别去停它。
 * 主循环想看时间了，读一下 Timer_GetTick() 就行。
 *
 * 填空顺序（一次只填一块，编译一次，烧录一次）：
 *   第 1 步  算预分频 PSC 和自动重装值 ARR，让中断周期正好 1ms
 *   第 2 步  打开更新中断 + NVIC 放行 + 启动定时器
 *   第 3 步  写 TIM2_IRQHandler，让 Tick 真的往上加
 *
 * 三块都填完，用调试器看 Timer_Tick，它应该每秒涨 1000。
 *
 * 参考答案在工程根目录的 _答案备份 文件夹里，卡住了再看。
 * ============================================================ */

/* 中断里改、主循环里读，必须加 volatile。
   不加的话编译器会认为"主循环里没人改它"，把你读到的值缓存进寄存器，
   于是主循环永远看到 0 —— 这种 bug 找起来要命。 */
static volatile uint32_t Timer_Tick = 0;


void Timer_Init(void)
{
	/* 下面这些是"送分题"，已经写好了 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	/* TIM2 挂在 APB1 上 */
	TIM_InternalClockConfig(TIM2);

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;

	/* ================= TODO 第 1 步 =================
	   把定时器调成"每 1ms 溢出一次"。

	   TIM2 的时钟是 72MHz，也就是每秒 72000000 个数。
	   先过预分频（PSC）把它降下来，再用重装值（ARR）数够就溢出。

	   建议拆成两段来想：
	     ① 先分频到 1MHz，这样每数 1 个数 = 1us
	        72MHz ÷ ? = 1MHz        → 分频系数 = 72
	     ② 1us 一个数，要凑够 1ms = 1000us
	        数够 ? 个数溢出一次     → 1000

	   ★ 陷阱：寄存器里存的不是"分频系数"，而是"分频系数 - 1"，
	     因为硬件是从 0 开始数的。所以这里要填 72-1 和 1000-1。

	   现在下面两行填的是 72 和 1000（没减 1），
	   不改也能跑，但实测会慢一点点 —— 正是那 1 个数的差距。
	================================================ */
	TIM_TimeBaseInitStructure.TIM_Period = 1000;		/* ← 改我：ARR */
	TIM_TimeBaseInitStructure.TIM_Prescaler = 72;		/* ← 改我：PSC */

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);

	/* ================= TODO 第 2 步 =================
	   让 TIM2 有资格"打断"CPU。按这个顺序写四小段：

	   ① 先清一次更新标志：TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	      （为什么：初始化过程中计数器可能已经溢出过一次，
	        标志还挂着，你一开中断它就立刻误进一次）

	   ② 打开更新中断：   TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	   ③ NVIC 里放行：
	      NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	      NVIC_InitTypeDef NVIC_InitStructure;
	      NVIC_InitStructure.NVIC_IRQChannel = ???;   ← TIM2 的中断通道叫什么？
	      NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	      NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	      NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	      NVIC_Init(&NVIC_InitStructure);

	   ④ 最后才启动：     TIM_Cmd(TIM2, ENABLE);
	      （顺序很重要：配置的时候别让它跑，配完再开）

	   通道名去 stm32f10x.h 里搜 "TIM2_IRQn" 就能确认。
	   照 EXTI 那章的思路来，只是把 EXTI0_IRQn 换成定时器的通道。
	================================================ */

	/* ← 第 2 步的代码写在这里 */

}


/**
  * @brief  读"开机以来过了多少毫秒"
  */
uint32_t Timer_GetTick(void)
{
	return Timer_Tick;
}


/* ================= TODO 第 3 步 =================
   写中断服务函数。

   函数名必须是 TIM2_IRQHandler —— 这个名字在启动文件 startup_stm32f10x_md.s
   里已经登记好了，写成别的（哪怕是 TIM2_IRQhandler 少个大写 H）
   编译不会报错，但中断永远进不去，你会盯着一个不动的秒表怀疑人生。

   函数体三句话：
     1) if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
     2) Timer_Tick ++;
     3) TIM_ClearITPendingBit(TIM2, TIM_IT_Update);   ← 忘了这句会反复进中断

   写完之后烧进去，用调试器（或者先用 main.c 里留的自检代码）
   看 Timer_Tick 是不是每秒涨 1000。
================================================ */
