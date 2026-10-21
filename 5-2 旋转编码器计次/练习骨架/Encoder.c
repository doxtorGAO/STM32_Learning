#include "stm32f10x.h"
#include "Encoder.h"

/* ============================================================
 * 旋转编码器计次 —— 填空练习版
 *
 * 和 5-1 最大的区别：
 *   5-1 用的是 EXTI15_10 这一条"多人共用"的中断通道；
 *   5-2 用的是 EXTI0 和 EXTI1 两条"独占"的通道，要开两个。
 *
 * 填空顺序（一次只填一块，编译一次，烧录一次）：
 *   第 2 步  打开 GPIOB 时钟 + 把 PB0、PB1 都配成上拉输入
 *   第 3 步  打开 AFIO 时钟 + 两条线分别映射 + 配 EXTI
 *   第 4 步  配置 NVIC，打开 EXTI0_IRQn 和 EXTI1_IRQn 两个通道
 *   第 5 步  写 Encoder_Get()，再用 main 把它显示出来（此时数字一直是 0）
 *   第 6 步  写两个中断服务函数，转动时数字才会变
 *
 * 参考答案在工程根目录的 _答案备份 文件夹里，卡住了再看。
 * ============================================================ */

/* 引脚与中断线的对应关系，写清楚免得下面看糊涂 */
#define ENCODER_GPIO		GPIOB
#define ENCODER_PIN_A		GPIO_Pin_0
#define ENCODER_PIN_B		GPIO_Pin_1
#define ENCODER_PIN_A_SRC	GPIO_PinSource0
#define ENCODER_PIN_B_SRC	GPIO_PinSource1
#define ENCODER_LINE_A		EXTI_Line0
#define ENCODER_LINE_B		EXTI_Line1

/* 中断里写、主循环里读的变量，必须加 volatile。
   这次用 int16_t 而不是 uint32_t，因为反转要出负数。 */
volatile int16_t Encoder_Count;


void Encoder_Init(void)
{
	/* TODO 第 2 步（1）：打开 GPIOB 的时钟 */

	/* TODO 第 2 步（2）：把 PB0 和 PB1 一起配成"上拉输入"
	   —— 提示：GPIO_Pin 可以用 | 把两个引脚拼起来，一次配完，
	      不用写两遍 GPIO_Init。 */

	/* TODO 第 3 步（1）：打开 AFIO 的时钟 */

	/* TODO 第 3 步（2）：把 EXTI0 接到 PB0、EXTI1 接到 PB1
	   —— 两条线要分别调用一次 GPIO_EXTILineConfig。 */

	/* TODO 第 3 步（3）：配置 EXTI0 和 EXTI1
	   —— 关键：EXTI_Line 可以用 | 把两条线拼起来一次配完
	      （EXTI_Line0 | EXTI_Line1）。
	   —— 四个成员和 5-1 一样，触发沿用 EXTI_Trigger_Falling。
	      编码器静止时两相都是高，转一格的过程中两相会先后拉低，
	      所以两个下降沿正好覆盖正转和反转。 */

	/* TODO 第 4 步：NVIC —— 这里是 5-2 最容易错的地方
	   —— 5-1 只有一个通道 EXTI15_10_IRQn；
	      5-2 有 0 号和 1 号两条线，各占一个通道：EXTI0_IRQn 和 EXTI1_IRQn，
	      要调用两次 NVIC_Init。
	   —— 抢占优先级都给 1，子优先级一个给 1 一个给 2（区分先后）。
	   —— 第二次调 NVIC_Init 之前，要把 NVIC_IRQChannel 改成 EXTI1_IRQn。 */

	/* 下面一行保留：上电时清零 */
	Encoder_Count = 0;
}


int16_t Encoder_Get(void)
{
	/* TODO 第 5 步：读一次就清零
	   思路：
	     1) 用局部变量 int16_t Temp 存下 Encoder_Count；
	     2) 把 Encoder_Count 清 0；
	     3) 返回 Temp。
	   为什么要清零：这个函数返回的是"自上次调用以来转了几格"，
	   不是累计值。main 里自己把每次的增量累加起来显示。
	   这样以后调参数（比如转一格亮度 +1）可以直接用返回值。 */
	return 0;
}


void EXTI0_IRQHandler(void)
{
	/* TODO 第 6 步（1）：A 相的下降沿
	   思路：
	     1) 判断是不是 LINE_A 触发的，是就立刻清挂起位；
	     2) 读 A 相现在是不是低电平（== 0），确认这是真边沿不是毛刺；
	     3) 再读 B 相：如果 B 也是低电平，说明 B 比 A 先变低，
	        这个方向是"反转"，Encoder_Count--；
	     4) 如果 B 是高电平，说明是正转 —— 这个方向不在这里处理，
	        交给 B 的下降沿去数，否则一次转动会被数两次。
	   提示：先清位还是先干活，回想 5-1 里讲过的理由。 */
}


void EXTI1_IRQHandler(void)
{
	/* TODO 第 6 步（2）：B 相的下降沿
	   和上面镜像对称：
	     1) 判断 LINE_B → 清挂起位；
	     2) 确认 B 现在是低电平；
	     3) 如果 A 也是低电平，说明 A 先变低 = 正转，Encoder_Count++。 */
}
