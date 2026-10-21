#include "stm32f10x.h"
#include "OLED.h"
#include "Key.h"
#include "Timer.h"
#include "Buzzer.h"

/* ============================================================
 * 5-3 按键计时器 —— 秒表 + 倒计时（双模式，2 个按键）
 *
 * 硬件：
 *   KEY1 = PB1   开始 / 暂停
 *   KEY2 = PB11  秒表模式：清零
 *                倒计时模式：停着按 = 换预设时长；跑着按 = 中止归零
 *                任何时候长按 1 秒 = 切换模式
 *   蜂鸣器 = PB12  倒计时到 0 响
 *   OLED  = PB8/PB9 软件 I2C
 *
 * 计时的核心思路（很重要）：
 *   TIM2 每 1ms 中断一次，中断里只做 Tick++。
 *   主循环每次读 Timer_GetTick()，和上次的值相减，把差值加到 ElapsedMs。
 *   这样即使刷屏花了很久、主循环某轮转得慢，时间差也是真实流逝的时间，
 *   走时不会因为"循环慢了"而变慢。
 *
 *   反例：如果用 Delay_ms(10); ElapsedMs += 10; 的写法，
 *   刷屏那几十毫秒会被漏掉，一分钟能慢好几秒。
 * ============================================================ */

/* ---- 模式 ---- */
#define MODE_STOPWATCH	0
#define MODE_COUNTDOWN	1

/* ---- 显示与提示的节拍 ---- */
#define REFRESH_MS		20			/* 每 20ms 刷新一次时间（OLED 软件 I2C 很慢，不能刷太勤） */
#define BLINK_MS		250			/* 到点后蜂鸣器通断周期 */
#define STOPWATCH_MAX_MS 5999990UL	/* 99:59.99，分钟位只有两位，到这儿就停 */

/* ---- 倒计时预设时长：KEY2 短按在这几档之间循环 ---- */
static const uint32_t PresetMs[] = { 30000, 60000, 120000, 300000, 600000 };
#define PRESET_NUM		(sizeof(PresetMs) / sizeof(PresetMs[0]))

/* ---- 运行状态 ---- */
static uint8_t	Mode        = MODE_STOPWATCH;	/* 当前模式 */
static uint8_t	Running     = 0;				/* 1=正在走，0=停着 */
static uint8_t	Done        = 0;				/* 1=倒计时已经归零，正在叫 */
static uint8_t	PresetIndex = 1;				/* 当前用第几档预设（默认 1 分钟） */
static uint32_t	ElapsedMs   = 0;				/* 已经走了多少毫秒 */
static uint32_t	LastTick    = 0;				/* 上一轮读到的 Tick */
static uint32_t	DoneTick    = 0;				/* 倒计时归零那一刻的 Tick */
static uint32_t	LastShow    = 0;				/* 上一次刷屏的 Tick */
static uint8_t	LastState   = 0xFF;				/* 上一次画的状态，0xFF 表示"需要重画" */

/* ---- 状态值（给 LastState 用） ---- */
#define ST_RUN		0
#define ST_PAUSE	1
#define ST_DONE		2

static void ShowTime(uint8_t Line, uint8_t Column, uint32_t Ms);
static void ShowSetTime(uint8_t Line, uint8_t Column, uint32_t Ms);
static uint32_t GetRemain(void);
static void DrawStatic(void);
static void DrawState(void);
static void HandleKey(uint8_t KeyNum);

/**
  * @brief  在指定位置显示 MM:SS.CC（共 8 个字符位）
  */
static void ShowTime(uint8_t Line, uint8_t Column, uint32_t Ms)
{
	OLED_ShowNum(Line, Column,     Ms / 60000, 2);			/* 分 */
	OLED_ShowChar(Line, Column + 2, ':');					/* 冒号 */
	OLED_ShowNum(Line, Column + 3, (Ms / 1000) % 60, 2);	/* 秒 */
	OLED_ShowChar(Line, Column + 5, '.');					/* 小数点 */
	OLED_ShowNum(Line, Column + 6, (Ms / 10) % 100, 2);		/* 百分秒 */
}

/**
  * @brief  在指定位置显示 MM:SS（共 5 个字符位），给"设定值"用
  */
static void ShowSetTime(uint8_t Line, uint8_t Column, uint32_t Ms)
{
	OLED_ShowNum(Line, Column,     Ms / 60000, 2);
	OLED_ShowChar(Line, Column + 2, ':');
	OLED_ShowNum(Line, Column + 3, (Ms / 1000) % 60, 2);
}

/**
  * @brief  倒计时还剩多少毫秒
  */
static uint32_t GetRemain(void)
{
	uint32_t Preset = PresetMs[PresetIndex];

	if (ElapsedMs >= Preset)
	{
		return 0;
	}
	return Preset - ElapsedMs;
}

/**
  * @brief  画不常变的内容：第 1 行模式名、第 4 行按键提示/设定值
  */
static void DrawStatic(void)
{
	if (Mode == MODE_STOPWATCH)
	{
		OLED_ShowString(1, 1, "STOPWATCH     ");
		OLED_ShowString(4, 1, "K1:GO K2:CLR  ");
	}
	else
	{
		OLED_ShowString(1, 1, "COUNTDOWN     ");
		OLED_ShowString(4, 1, "SET ");
		ShowSetTime(4, 5, PresetMs[PresetIndex]);
		OLED_ShowString(4, 10, " K2:SET  ");
	}
}

/**
  * @brief  画第 3 行的运行状态，状态没变就不刷（省时间）
  */
static void DrawState(void)
{
	uint8_t State;

	if ((Mode == MODE_COUNTDOWN) && (Done == 1))
	{
		State = ST_DONE;
	}
	else if (Running)
	{
		State = ST_RUN;
	}
	else
	{
		State = ST_PAUSE;
	}

	if (State == LastState)
	{
		return;
	}
	LastState = State;

	if (State == ST_RUN)
	{
		OLED_ShowString(3, 1, "> RUN       ");
	}
	else if (State == ST_PAUSE)
	{
		OLED_ShowString(3, 1, "> PAUSE     ");
	}
	else
	{
		OLED_ShowString(3, 1, "> TIME UP!  ");
	}
}

/**
  * @brief  处理一个按键事件
  */
static void HandleKey(uint8_t KeyNum)
{
	uint8_t WasRunning;

	if (KeyNum == KEY_NONE)
	{
		return;
	}

	/* ---- KEY2 长按：切换模式 ---- */
	if (KeyNum == KEY2_LONG)
	{
		Running = 0;
		Done = 0;
		ElapsedMs = 0;
		LastState = 0xFF;
		Buzzer_OFF();

		Mode = (Mode == MODE_STOPWATCH) ? MODE_COUNTDOWN : MODE_STOPWATCH;

		OLED_Clear();			/* 换模式了，整屏重画 */
		DrawStatic();
		DrawState();
		return;
	}

	/* ---- KEY1 短按：开始 / 暂停（到点后按它 = 重来） ---- */
	if (KeyNum == KEY1_SHORT)
	{
		if (Done)
		{
			Done = 0;
			ElapsedMs = 0;
			Running = 1;
			Buzzer_OFF();
		}
		else
		{
			Running = !Running;
		}
		return;
	}

	/* ---- KEY2 短按 ---- */
	if (KeyNum == KEY2_SHORT)
	{
		if (Mode == MODE_STOPWATCH)
		{
			/* 秒表：清零并停表 */
			Running = 0;
			ElapsedMs = 0;
		}
		else
		{
			/* 倒计时：先记住原来是不是在跑 */
			WasRunning = Running;
			Running = 0;
			Done = 0;
			ElapsedMs = 0;
			Buzzer_OFF();

			if (WasRunning == 0)
			{
				/* 本来就没跑 → 换下一档预设时长 */
				PresetIndex = (uint8_t)((PresetIndex + 1) % PRESET_NUM);
				DrawStatic();		/* 设定值变了，第 4 行要重画 */
			}
			/* 本来在跑 → 只中止归零，时长不变 */
		}
		return;
	}
}

int main(void)
{
	uint32_t Now;
	uint32_t Preset;
	uint8_t KeyNum;

	OLED_Init();
	Key_Init();
	Buzzer_Init();
	Timer_Init();		/* 最后开定时器，前面的外设都准备好了再让心跳开始 */

	DrawStatic();
	DrawState();

	LastTick = Timer_GetTick();
	LastShow = LastTick;

	while (1)
	{
		Now = Timer_GetTick();

		/* ---- 1. 计时：把"这次和上次的时间差"累加进来 ---- */
		if (Running)
		{
			if (Mode == MODE_STOPWATCH)
			{
				if (ElapsedMs < STOPWATCH_MAX_MS)
				{
					ElapsedMs += Now - LastTick;
					if (ElapsedMs > STOPWATCH_MAX_MS)
					{
						ElapsedMs = STOPWATCH_MAX_MS;	/* 分钟位放不下了，停在 99:59.99 */
					}
				}
			}
			else
			{
				ElapsedMs += Now - LastTick;
			}
		}
		LastTick = Now;

		/* ---- 2. 倒计时到点判定 ---- */
		if ((Mode == MODE_COUNTDOWN) && Running)
		{
			Preset = PresetMs[PresetIndex];
			if (ElapsedMs >= Preset)
			{
				ElapsedMs = Preset;
				Running = 0;
				Done = 1;
				DoneTick = Now;
			}
		}

		/* ---- 3. 扫按键（非阻塞，扫一眼就走） ---- */
		KeyNum = Key_GetNum();
		if (KeyNum != KEY_NONE)
		{
			HandleKey(KeyNum);
			Now = Timer_GetTick();		/* 处理按键可能花了几毫秒，刷新一下 Now */
			/* 注意：这里不要再改 LastTick。
			   第 1 步已经把它对齐到本轮开头了，
			   再改一次就等于把"处理按键耗掉的时间"从计时里抹掉了。 */
		}

		/* ---- 4. 到点了就让蜂鸣器断续叫 ---- */
		if (Done)
		{
			if (((Now - DoneTick) / BLINK_MS) % 2 == 0)
			{
				Buzzer_ON();
			}
			else
			{
				Buzzer_OFF();
			}
		}
		else
		{
			Buzzer_OFF();
		}

		/* ---- 5. 刷屏：每 20ms 刷一次时间，别刷太勤 ---- */
		if ((Now - LastShow) >= REFRESH_MS)
		{
			LastShow = Now;

			if (Mode == MODE_STOPWATCH)
			{
				ShowTime(2, 4, ElapsedMs);
			}
			else
			{
				ShowTime(2, 4, GetRemain());
			}
			DrawState();
		}
	}
}
