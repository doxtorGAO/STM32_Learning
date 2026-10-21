#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "Key.h"
#include "CountSensor.h"

/* ============================================================
 * 对射式红外传感器计次 —— main.c 填空练习版
 *
 * 流程：初始化 -> 循环里 {消抖确认、刷新显示、查按键}
 * ============================================================ */

int main(void)
{
	OLED_Init();
	Key_Init();
	CountSensor_Init();

	OLED_ShowString(1, 1, "Count:");
	OLED_ShowString(3, 1, "KEY1: Reset");

	while (1)
	{
		/* TODO 第 6 步：调用 CountSensor_Update() ，让它在主循环里反复做消抖确认
		CountSensor_Update();
		*/

		/* ---- 第 2 步的自测代码（用来确认接线和极性对不对）----
		   打开下面这一行，烧录后：
		     手没挡光 = 显示 1，手挡住 = 显示 0。
		   看到变化，说明"接线 + 模块 + 读引脚"全对了，可以去做第 3 步。
		   验证通过后，把这一行重新注释掉，改用第 5 步的计数值显示。
		OLED_ShowNum(2, 1, GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14), 1);
		*/

		/* TODO 第 5 步：显示计数值
		   提示：OLED_ShowNum(行, 列, 数值, 显示位数)
		   用 10 位显示，这样 uint32_t 的计数值不会像 16 位那样数到 65535 就回绕。
		OLED_ShowNum(2, 1, CountSensor_Get(), 10);
		*/

		/* TODO 第 7 步：按键清零
		   提示：
		     1) 先在函数开头（while 之前）声明一个 uint8_t KeyNum; 存按键返回值；
		     2) 这里调用 KeyNum = Key_GetNum();
		     3) 如果 KeyNum == 1（KEY1 被按下），就 CountSensor_Reset();，
		        并且立刻 OLED_ShowNum(2, 1, 0, 10); 把屏幕也刷成 0。
		*/
	}
}