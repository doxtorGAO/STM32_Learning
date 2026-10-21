#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "CountSensor.h"

/* ============================================================
 * 对射式红外传感器计次 —— 填空练习版
 *
 * 填空顺序（一次只填一块，编译一次，烧录一次，确认现象对了再往下）：
 *   第 2 步  打开 GPIOB 时钟 + 把 PB14 配成上拉输入
 *   第 3 步  打开 AFIO 时钟 + 接线到 EXTI14 + 配置 EXTI
 *   第 4 步  配置 NVIC，打开 EXTI15_10_IRQn
 *   第 5 步  写中断服务函数，先直接 Count++（先让它"能数"）
 *   第 6 步  改成"标志 + CountSensor_Update()"两段式消抖（数得准）
 *
 * 参考答案在工程根目录的 _答案备份 文件夹里，卡住了再看。
 * ============================================================ */

/* 你的模块：没遮挡输出高、被遮挡输出低。
   触发边沿和电平确认必须自洽：
     被遮挡那一瞬间是"高 -> 低"，所以用 EXTI_Trigger_Falling，
     确认时引脚应该是低电平，所以 ACTIVE_LEVEL = 0。
   如果哪天换的模块极性相反，这两行必须同时改。 */
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
	/* TODO 第 2 步（1）：打开 GPIOB 的时钟
	   —— 单片机默认把外设时钟关着省电，不开时钟，后面配置全部无效。 */

	/* TODO 第 2 步（2）：把 PB14 配成"上拉输入"
	   —— 四件套：GPIO_Mode = GPIO_Mode_IPU、GPIO_Pin = GPIO_Pin_14、
	              GPIO_Speed = GPIO_Speed_50MHz、然后 GPIO_Init(GPIOB, &结构体) */

	/* TODO 第 3 步（1）：打开 AFIO 的时钟
	   —— 引脚要接到中断线上，必须开这个。 */

	/* TODO 第 3 步（2）：告诉它 EXTI14 这条线来自 GPIOB 的 14 号脚
	   —— 用 GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
	   —— 注意：EXTI 是按"引脚号"排的，PA14/PB14/PC14 抢同一条线，
	      所以这一步是必须的，不能省。 */

	/* TODO 第 3 步（3）：配置 EXTI14
	   —— 四个成员：EXTI_Line = EXTI_Line14、EXTI_LineCmd = ENABLE、
	              EXTI_Mode = EXTI_Mode_Interrupt、EXTI_Trigger = COUNT_SENSOR_TRIGGER_EDGE、
	              最后 EXTI_Init(&结构体)。 */

	/* TODO 第 4 步：中断优先级 + 打开中断通道
	   —— NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	   —— NVIC 结构体四个成员：NVIC_IRQChannel = EXTI15_10_IRQn（注意不是 EXTI14）、
	      NVIC_IRQChannelCmd = ENABLE、抢占优先级、子优先级，最后 NVIC_Init(&结构体)。 */

	/* 下面两行保留：上电时清零 */
	CountSensor_Flag = 0;
	CountSensor_Count = 0;
}


void CountSensor_Update(void)
{
	/* TODO 第 6 步：这里是消抖确认的地方，在主循环里反复被调用。
	   思路：
	     1) 如果 CountSensor_Flag 为 1，说明刚检测到边沿；
	     2) Delay_ms(COUNT_SENSOR_DEBOUNCE_MS) 等电平稳定；
	     3) 把 Flag 清 0（抖动期间的额外边沿顺便丢掉）；
	     4) 再读一次引脚，如果 == COUNT_SENSOR_ACTIVE_LEVEL，说明确实被挡住了，
	        CountSensor_Count++，然后再 Delay_ms 一小段当"静默期"，
	        防止同一段遮挡被重复计数；
	     5) 把 Flag 再清一次。
	   为什么不在中断里做：Delay_ms 是独占 SysTick 忙等的，
	   在中断里调用会打乱主循环的延时，甚至让主循环卡死。 */
}


void EXTI15_10_IRQHandler(void)
{
	/* TODO 第 5 步：函数名不能改！启动文件里登记的就是这个名字，改了就永远进不来。
	   先写"能数"的简单版本：
	     if (EXTI_GetITStatus(EXTI_Line14) == SET)
	     {
	         CountSensor_Count++;                  // 来一次加一
	         EXTI_ClearITPendingBit(EXTI_Line14); // 必须清标志，否则出不去了
	     }

	   第 6 步再把它改成"只置标志"：
	     if (EXTI_GetITStatus(EXTI_Line14) == SET)
	     {
	         EXTI_ClearITPendingBit(EXTI_Line14);
	         CountSensor_Flag = 1;                 // 只记一笔，立刻退出
	     } */
}


uint32_t CountSensor_Get(void)
{
	/* TODO：返回计数值（很简单，可以先填） */
	return 0;
}


void CountSensor_Reset(void)
{
	/* TODO：把计数值清零 */
}