#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "CountSensor.h"


int main(void)
{
	OLED_Init();
	Key_Init();
	CountSensor_Init();
	OLED_ShowString(1, 1, "Count:");

	while (1)
	{
		CountSensor_Update();
		OLED_ShowNum(2, 1, CountSensor_Get(), 10);
		
	}
}