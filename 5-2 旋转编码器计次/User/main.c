#include "stm32f10x.h"
#include "OLED.h"
#include "Encoder.h"

/* ============================================================
 * 旋转编码器计次 —— main.c 填空练习版
 *
 * 注意：这个工程不要调 Key_Init()。
 *       5-1 的 Key.c 用的 PB1 正是编码器的 B 相，两个模块抢同一个引脚。
 *       所以这里连 Key.h 都不 include。
 * ============================================================ */

int main(void)
{
	int16_t Num = 0;		/* 累计值，纯显示用 */

	OLED_Init();
	Encoder_Init();

	OLED_ShowString(1, 1, "Num:");
	OLED_ShowString(3, 1, "CW+ CCW-");

	while (1)
	{
		Num += Encoder_Get();
		OLED_ShowSignedNum(2,1,Num,5);
		OLED_ShowNum(4, 1, GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0), 1);
OLED_ShowNum(4, 3, GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1), 1);

		/* TODO 第 5 步：把增量累加进来再显示
		   提示：
		     1) Num += Encoder_Get();   拿到的是"这一圈转了几格"
		     2) OLED_ShowSignedNum(2, 1, Num, 5);
		   为什么用 ShowSignedNum 而不是 ShowNum：
		     反转时 Num 会变成负数，ShowNum 只认无符号数，
		     传负数进去会显示成一个巨大的正数。 */
	}
}
