#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "CountSensor.h"

int main(void)
{
	uint8_t KeyNum;

	OLED_Init();
	Key_Init();
	CountSensor_Init();

	OLED_ShowString(1, 1, "Count:");
	OLED_ShowString(3, 1, "KEY1: Reset");

	while (1)
	{
		CountSensor_Update();						/* 主循环里做消抖确认与计数 */

		OLED_ShowNum(2, 1, CountSensor_Get(), 10);	/* uint32_t 全量程 10 位显示，不会在 65535 回绕 */

		KeyNum = Key_GetNum();						/* KEY1(PB1) 按下清零；KEY2(PB11) 暂未使用 */
		if (KeyNum == 1)
		{
			CountSensor_Reset();
			OLED_ShowNum(2, 1, 0, 10);
		}
	}
}
