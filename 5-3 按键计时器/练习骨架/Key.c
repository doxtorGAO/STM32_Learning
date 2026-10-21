#include "stm32f10x.h"
#include "Key.h"
#include "Timer.h"

/* ============================================================
 * Key.c —— 非阻塞按键扫描（填空练习版）
 *
 * 和你 3-4 写的 Key.c 最大的不同：
 *   旧的：while (引脚 == 0);   ← 死等松手，按住键主循环就卡死
 *   新的：读一次电平马上返回，靠"记住了什么时刻变的"来判断按键
 *
 * 为什么要改：这个工程里主循环一卡，OLED 不刷新、蜂鸣器不响、
 * 你按住键的时候秒表跟死了一样。计时器不能有死等。
 *
 * 填空顺序：
 *   第 4 步  GPIO 初始化
 *   第 5 步  消抖 + 短按
 *   第 6 步  长按
 * ============================================================ */

#define KEY_NUM			2
#define KEY_DEBOUNCE_MS	20		/* 消抖时间：稳定 20ms 才算数 */
#define KEY_LONG_MS		1000	/* 长按判定：按住 1000ms 算长按 */

/* 两个键对应的引脚：KEY1 = PB1，KEY2 = PB11 */
static const uint16_t Key_Pin[KEY_NUM] = { GPIO_Pin_1, GPIO_Pin_11 };

/* 每个键 5 个"记忆" */
static uint8_t	Key_Raw[KEY_NUM];		/* 上一次采到的原始电平 */
static uint8_t	Key_Stable[KEY_NUM];	/* 消抖之后的稳定电平，1=松开 0=按下 */
static uint8_t	Key_LongSent[KEY_NUM];	/* 这次按住期间，长按事件是否已上报过 */
static uint32_t	Key_ChangeTick[KEY_NUM];/* 原始电平最后一次变化的时刻 */
static uint32_t	Key_PressTick[KEY_NUM];	/* 稳定按下的时刻 */


void Key_Init(void)
{
	uint8_t i;

	/* ================= TODO 第 4 步 =================
	   和 3-4 一模一样，只是这次两个引脚一起配：
	     RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	     GPIO_Mode  = GPIO_Mode_IPU      （上拉输入：不按读回来是 1）
	     GPIO_Pin   = GPIO_Pin_1 | GPIO_Pin_11
	     GPIO_Speed = GPIO_Speed_50MHz
	     GPIO_Init(GPIOB, &GPIO_InitStructure);
	================================================ */

	/* ← 第 4 步的代码写在这里 */


	/* 下面这段别删：上电时把所有"记忆"都当成"松开"。
	   如果漏了，Key_Stable 初值是 0（=按下），
	   主循环第一轮就会误判你在按键。 */
	for (i = 0; i < KEY_NUM; i++)
	{
		Key_Raw[i] = 1;
		Key_Stable[i] = 1;
		Key_LongSent[i] = 0;
		Key_ChangeTick[i] = 0;
		Key_PressTick[i] = 0;
	}
}


uint8_t Key_GetNum(void)
{
	uint8_t Event = KEY_NONE;
	uint8_t i;
	uint8_t Level;
	uint32_t Now = Timer_GetTick();

	for (i = 0; i < KEY_NUM; i++)
	{
		Level = GPIO_ReadInputDataBit(GPIOB, Key_Pin[i]);

		/* ================= TODO 第 5 步（1）=================
		   刚读到的 Level 和上次记的 Key_Raw[i] 不一样，
		   说明"可能"在抖动，也可能真的按下/松开了。
		   这时要做的事：把 Key_Raw[i] 更新成 Level，
		   并且把 Key_ChangeTick[i] 记成现在的 Now。

		   作用：后面就靠这个时刻判断"这个电平已经稳定多久了"。
		==================================================== */

		/* ================= TODO 第 5 步（2）=================
		   消抖 + 出事件。两层判断，别漏：

		   if (Key_Raw[i] != Key_Stable[i])
		   并且 (Now - Key_ChangeTick[i]) >= KEY_DEBOUNCE_MS
		   {
		       Key_Stable[i] = Key_Raw[i];      ← 承认它变了

		       if (Key_Stable[i] == 0)          ← 刚按下
		       {
		           Key_PressTick[i] = Now;      ← 记下按下去的时刻（第 6 步要用）
		           Key_LongSent[i] = 0;         ← 清长按标记
		       }
		       else                             ← 刚松开
		       {
		           if (Key_LongSent[i] == 0)    ← 中途没报过长按，才算短按
		           {
		               Event = (i == 0) ? KEY1_SHORT : KEY2_SHORT;
		           }
		       }
		   }

		   为什么短按在"松开"时报，而不是"按下"时报：
		   因为按下的那一刻你还不知道用户是短按还是长按，
		   必须等松手了才知道"哦，原来只按了一下"。
		==================================================== */

		/* ================= TODO 第 6 步 =================
		   长按：按住不放满 1 秒，报一次 KEY2_LONG。

		   条件（三个都要满足）：
		     Key_Stable[i] == 0            ← 现在是按着的
		     Key_LongSent[i] == 0          ← 这次按住还没报过长按
		     (Now - Key_PressTick[i]) >= KEY_LONG_MS   ← 从按下到现在够 1 秒了

		   满足就：
		     Key_LongSent[i] = 1;          ← 标记已上报，避免每轮都报
		     if (i == 1) Event = KEY2_LONG;   ← 长按功能只给 KEY2

		   为什么要有 Key_LongSent 这个标记：
		   不然主循环转得飞快，1 秒之后每一轮都满足条件，
		   你会收到成百上千个长按事件，模式瞬间被切换无数次。
		================================================ */
	}

	return Event;
}
