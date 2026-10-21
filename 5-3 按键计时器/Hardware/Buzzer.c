#include "stm32f10x.h"

/* ============================================================
 * Buzzer.c —— 蜂鸣器（PB12）
 * 和你 3-3 蜂鸣器工程是同一个引脚：低电平响，高电平停
 * ============================================================ */

/**
  * @brief  蜂鸣器初始化
  * @param  无
  * @retval 无
  */
void Buzzer_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_SetBits(GPIOB, GPIO_Pin_12);		/* 上电默认别叫 */
}

/**
  * @brief  蜂鸣器响
  * @param  无
  * @retval 无
  */
void Buzzer_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_12);
}

/**
  * @brief  蜂鸣器停
  * @param  无
  * @retval 无
  */
void Buzzer_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
}
