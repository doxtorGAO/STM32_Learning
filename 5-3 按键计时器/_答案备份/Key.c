#include "stm32f10x.h"
#include "Key.h"
#include "Timer.h"

/* ============================================================
 * Key.c —— 非阻塞按键扫描（支持短按 / 长按）
 *
 * 为什么不用 3-4 那个 Key_GetNum() 了？
 *   老的写法里有 while (引脚 == 0);  ← 等松手，是死等。
 *   你按住键不放，主循环就卡在那儿，OLED 不刷新、计时也停。
 *   计时器最忌讳这个，所以按键必须"扫一眼就走"。
 *
 * 新的做法：
 *   1) 每次调用只读一下当前电平，记下"电平最后一次变化的时刻"；
 *   2) 电平稳定超过 20ms 才承认它真的变了（消抖）；
 *   3) 按下时记下时刻，按住满 1 秒就报一次长按；
 *   4) 松开时如果没报过长按，就报一次短按。
 *   全程没有任何死等，函数瞬间返回。
 * ============================================================ */

#define KEY_NUM			2
#define KEY_DEBOUNCE_MS	20		/* 消抖时间：稳定 20ms 才算数 */
#define KEY_LONG_MS		1000	/* 长按判定：按住 1000ms 算长按 */

/* 两个键对应的引脚：KEY1 = PB1，KEY2 = PB11 */
static const uint16_t Key_Pin[KEY_NUM] = { GPIO_Pin_1, GPIO_Pin_11 };

static uint8_t	Key_Raw[KEY_NUM];		/* 上一次采到的原始电平 */
static uint8_t	Key_Stable[KEY_NUM];	/* 消抖之后的稳定电平，1=松开 0=按下 */
static uint8_t	Key_LongSent[KEY_NUM];	/* 这次按住期间，长按事件是否已上报过 */
static uint32_t	Key_ChangeTick[KEY_NUM];/* 原始电平最后一次变化的时刻 */
static uint32_t	Key_PressTick[KEY_NUM];	/* 稳定按下的时刻 */

/**
  * @brief  按键初始化：PB1、PB11 上拉输入
  * @param  无
  * @retval 无
  */
void Key_Init(void)
{
	uint8_t i;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;		/* 上拉输入：不按的时候读回来是 1 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* 初值都按"松开"处理，不然后面第一次扫描会误判成按下 */
	for (i = 0; i < KEY_NUM; i++)
	{
		Key_Raw[i] = 1;
		Key_Stable[i] = 1;
		Key_LongSent[i] = 0;
		Key_ChangeTick[i] = 0;
		Key_PressTick[i] = 0;
	}
}

/**
  * @brief  扫描按键，返回一个事件
  * @param  无
  * @retval KEY_NONE / KEY1_SHORT / KEY2_SHORT / KEY2_LONG
  * @note   要在主循环里反复调用。它不会阻塞，读一次就返回。
  */
uint8_t Key_GetNum(void)
{
	uint8_t Event = KEY_NONE;
	uint8_t i;
	uint8_t Level;
	uint32_t Now = Timer_GetTick();

	for (i = 0; i < KEY_NUM; i++)
	{
		Level = GPIO_ReadInputDataBit(GPIOB, Key_Pin[i]);

		/* ---- 第 1 步：电平一变，就把"变化时刻"刷新到现在 ---- */
		if (Level != Key_Raw[i])
		{
			Key_Raw[i] = Level;
			Key_ChangeTick[i] = Now;
		}

		/* ---- 第 2 步：稳定够 20ms，才承认状态真的变了 ---- */
		if ((Key_Raw[i] != Key_Stable[i]) && ((Now - Key_ChangeTick[i]) >= KEY_DEBOUNCE_MS))
		{
			Key_Stable[i] = Key_Raw[i];

			if (Key_Stable[i] == 0)
			{
				/* 刚按下：记时刻，清长按标记 */
				Key_PressTick[i] = Now;
				Key_LongSent[i] = 0;
			}
			else
			{
				/* 刚松开：只要中途没报过长按，就是一次短按 */
				if (Key_LongSent[i] == 0)
				{
					Event = (i == 0) ? KEY1_SHORT : KEY2_SHORT;
				}
			}
		}

		/* ---- 第 3 步：一直按着不放，满 1 秒报一次长按 ---- */
		if ((Key_Stable[i] == 0) && (Key_LongSent[i] == 0) && ((Now - Key_PressTick[i]) >= KEY_LONG_MS))
		{
			Key_LongSent[i] = 1;
			if (i == 1)			/* 长按功能只给 KEY2 */
			{
				Event = KEY2_LONG;
			}
		}
	}

	return Event;
}
