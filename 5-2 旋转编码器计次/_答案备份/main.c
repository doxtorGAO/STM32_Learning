#include "stm32f10x.h"
#include "OLED.h"
#include "Encoder.h"

/* ============================================================
 * 旋转编码器计次 —— main.c 参考答案
 *
 * 不调 Key_Init()：5-1 的 Key.c 用的 PB1 正是编码器 B 相，会打架。
 * ============================================================ */

int main(void)
{
	int16_t Num = 0;

	OLED_Init();
	Encoder_Init();

	OLED_ShowString(1, 1, "Num:");
	OLED_ShowString(3, 1, "CW+ CCW-");

	while (1)
	{
		Num += Encoder_Get();				/* 累加增量 */
		OLED_ShowSignedNum(2, 1, Num, 5);	/* 有符号显示，能看到负号 */
	}
}
