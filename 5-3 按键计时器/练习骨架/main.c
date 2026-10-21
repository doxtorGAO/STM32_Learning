#include "stm32f10x.h"
#include "OLED.h"
#include "Key.h"
#include "Timer.h"
#include "Buzzer.h"

/* ============================================================
 * 5-3 按键计时器 —— main.c（填空练习版）
 *
 * 这个文件里，显示、方案切换那些"胶水代码"都给你写好了，
 * 你只需要填两块：
 *   第 7 步  计时累加 + 按键分发   （填完秒表就能用）
 *   第 8 步  倒计时到点 + 蜂鸣器   （填完倒计时才完整）
 *
 * 建议顺序：先把 Timer.c 三块、Key.c 三块填完，再来这里。
 * 前面的没填好，这里填了也没反应。
 *
 * 硬件：
 *   KEY1 = PB1   开始 / 暂停
 *   KEY2 = PB11  秒表：清零 | 倒计时：换档/中止 | 长按 1 秒：切模式
 *   蜂鸣器 = PB12（低电平响）
 * ============================================================ */

#define MODE_STOPWATCH	0
#define MODE_COUNTDOWN	1

#define REFRESH_MS		20			/* 每 20ms 刷一次时间（OLED 软件 I2C 慢，别刷太勤） */
#define BLINK_MS		250			/* 到点后蜂鸣器通断周期 */
#define STOPWATCH_MAX_MS 5999990UL	/* 99:59.99，分钟位只有两位，到这儿就停 */

/* 倒计时预设时长：KEY2 短按在这几档之间循环 */
static const uint32_t PresetMs[] = { 30000, 60000, 120000, 300000, 600000 };
#define PRESET_NUM		(sizeof(PresetMs) / sizeof(PresetMs[0]))

/* 运行状态 */
static uint8_t	Mode        = MODE_STOPWATCH;	/* 当前模式 */
static uint8_t	Running     = 0;				/* 1=正在走，0=停着 */
static uint8_t	Done        = 0;				/* 1=倒计时已经归零，正在叫 */
static uint8_t	PresetIndex = 1;				/* 当前用第几档预设（默认 1 分钟） */
static uint32_t	ElapsedMs   = 0;				/* 已经走了多少毫秒 */
static uint32_t	LastTick    = 0;				/* 上一轮读到的 Tick */
static uint32_t	DoneTick    = 0;				/* 倒计时归零那一刻的 Tick */
static uint32_t	LastShow    = 0;				/* 上一次刷屏的 Tick */
static uint8_t	LastState   = 0xFF;				/* 上一次画的状态，0xFF = 需要重画 */

#define ST_RUN		0
#define ST_PAUSE	1
#define ST_DONE		2

static void ShowTime(uint8_t Line, uint8_t Column, uint32_t Ms);
static void ShowSetTime(uint8_t Line, uint8_t Column, uint32_t Ms);
static uint32_t GetRemain(void);
static void DrawStatic(void);
static void DrawState(void);
static void HandleKey(uint8_t KeyNum);

/* ---------- 以下是写好的显示代码，不用改 ---------- */

static void ShowTime(uint8_t Line, uint8_t Column, uint32_t Ms)
{
	OLED_ShowNum(Line, Column,     Ms / 60000, 2);			/* 分 */
	OLED_ShowChar(Line, Column + 2, ':');					/* 冒号 */
	OLED_ShowNum(Line, Column + 3, (Ms / 1000) % 60, 2);	/* 秒 */
	OLED_ShowChar(Line, Column + 5, '.');					/* 小数点 */
	OLED_ShowNum(Line, Column + 6, (Ms / 10) % 100, 2);		/* 百分秒 */
}

static void ShowSetTime(uint8_t Line, uint8_t Column, uint32_t Ms)
{
	OLED_ShowNum(Line, Column,     Ms / 60000, 2);
	OLED_ShowChar(Line, Column + 2, ':');
	OLED_ShowNum(Line, Column + 3, (Ms / 1000) % 60, 2);
}

static uint32_t GetRemain(void)
{
	uint32_t Preset = PresetMs[PresetIndex];

	if (ElapsedMs >= Preset)
	{
		return 0;
	}
	return Preset - ElapsedMs;
}

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

	if (State == LastState)			/* 状态没变就别刷，省时间 */
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

/* ---------- 按键处理：也是写好的，不用改 ---------- */
/* 它只负责"根据事件改状态"，不负责判断按键长短，那事在 Key.c 里 */

static void HandleKey(uint8_t KeyNum)
{
	uint8_t WasRunning;

	if (KeyNum == KEY_NONE)
	{
		return;
	}

	if (KeyNum == KEY2_LONG)					/* 长按：切换模式 */
	{
		Running = 0;
		Done = 0;
		ElapsedMs = 0;
		LastState = 0xFF;
		Buzzer_OFF();

		Mode = (Mode == MODE_STOPWATCH) ? MODE_COUNTDOWN : MODE_STOPWATCH;

		OLED_Clear();
		DrawStatic();
		DrawState();
		return;
	}

	if (KeyNum == KEY1_SHORT)					/* KEY1：开始 / 暂停 */
	{
		if (Done)								/* 响完了，再按就重来 */
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

	if (KeyNum == KEY2_SHORT)					/* KEY2 短按 */
	{
		if (Mode == MODE_STOPWATCH)
		{
			Running = 0;						/* 秒表：清零并停表 */
			ElapsedMs = 0;
		}
		else
		{
			WasRunning = Running;				/* 先记住原来是不是在跑 */
			Running = 0;
			Done = 0;
			ElapsedMs = 0;
			Buzzer_OFF();

			if (WasRunning == 0)				/* 本来没跑 → 换下一档 */
			{
				PresetIndex = (uint8_t)((PresetIndex + 1) % PRESET_NUM);
				DrawStatic();
			}
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
	Timer_Init();			/* 最后开定时器：外设都就绪了再让心跳开始 */

	DrawStatic();
	DrawState();

	LastTick = Timer_GetTick();
	LastShow = LastTick;

	while (1)
	{
		Now = Timer_GetTick();

		/* ================= TODO 第 7 步（1）：累加时间 =================
		   如果 Running 为 1，说明正在走，把这一轮流逝的时间加进去：
		       ElapsedMs += Now - LastTick;

		   秒表模式还要防止分钟位溢出（最多 99:59.99）：
		       if (ElapsedMs > STOPWATCH_MAX_MS) ElapsedMs = STOPWATCH_MAX_MS;

		   ★ 不管跑不跑，最后都要写一句：
		       LastTick = Now;
		     为什么要无条件更新：如果你只在跑的时候更新，
		     那么暂停 10 秒再开始，这 10 秒会被算进去，
		     一恢复就发现数字凭空跳了 10 秒。

		   ★ 为什么不直接 ElapsedMs++ 或者加 Delay_ms 的时长：
		     每轮循环耗时不固定（刷屏那轮要好几毫秒），
		     加固定值会让走时忽快忽慢。
		     用"两个时间戳相减"，加的就是真实流逝的时间，永远准。
		============================================================== */

		/* ← 第 7 步（1）写在这里 */


		/* ================= TODO 第 7 步（2）：扫按键 =================
		       KeyNum = Key_GetNum();
		       if (KeyNum != KEY_NONE)
		       {
		           HandleKey(KeyNum);
		           Now = Timer_GetTick();   ← 处理按键花了几毫秒，更新一下 Now
		       }

		   ★ 注意：这里不要再写 LastTick = Now;
		     上面第（1）步已经对齐过了，再改一次等于把
		     "处理按键耗掉的时间"从计时里抹掉。
		============================================================== */

		/* ← 第 7 步（2）写在这里 */


		/* ================= TODO 第 8 步（1）：倒计时到点 =================
		   只在倒计时模式、并且正在跑的时候判断：
		       Preset = PresetMs[PresetIndex];
		       if (ElapsedMs >= Preset)
		       {
		           ElapsedMs = Preset;    ← 别让它超过，显示会是负的/跳格
		           Running = 0;
		           Done = 1;
		           DoneTick = Now;        ← 记下归零时刻，蜂鸣器要用
		       }
		================================================================ */

		/* ← 第 8 步（1）写在这里 */


		/* ================= TODO 第 8 步（2）：蜂鸣器 =================
		   到点了就断续叫，用 DoneTick 算已经过了几个 250ms：
		       if (Done)
		       {
		           if (((Now - DoneTick) / BLINK_MS) % 2 == 0) Buzzer_ON();
		           else                                        Buzzer_OFF();
		       }
		       else
		       {
		           Buzzer_OFF();
		       }

		   ★ 这里也绝对不能写 Delay_ms()。
		     一旦延时，主循环就停在那儿，按键没反应、显示冻结。
		     想要"过一会儿再做什么"，就用时间戳比大小，
		     让主循环一直转，转到了才做 —— 这叫非阻塞延时。
		================================================================ */

		/* ← 第 8 步（2）写在这里 */


		/* ---------- 刷屏：写好的，不用改 ---------- */
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

		/* ---------- 临时自检：验证时基对不对 ----------
		   第 3 步做完（Timer.c 全填完）之后，
		   把下面两行的注释去掉，烧进去看第 4 行的数字：
		     每秒涨 1000 → 时基正确
		     每秒涨 999 左右 → 你第 1 步应该漏了 "减 1"
		     完全不动 → 中断没进，回去看第 2、3 步
		   验完记得把注释加回去，否则会盖住第 4 行的按键提示。
		*/
		/* OLED_ShowNum(4, 1, Timer_GetTick() / 1000, 5); */
	}
}
