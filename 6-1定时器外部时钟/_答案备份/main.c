#include "stm32f10x.h"
#include "OLED.h"
#include "Timer.h"

int main(void)
{
	OLED_Init();
	Timer_Init();

	OLED_ShowString(1, 1, "Num:");

	while (1)
	{
		OLED_ShowNum(2, 1, Num, 5);
	}
}
