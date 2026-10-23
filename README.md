# STM32 学习记录

跟着江科大（江协科技）STM32 入门教程逐个外设实践的过程记录，主控 STM32F103C8T6，使用 Keil MDK + 标准外设库。

每个子目录是一个独立的 Keil 工程，可以直接打开里面的 `Project.uvprojx` 编译。

## 工程目录

| 目录 | 内容 | 涉及外设 |
| --- | --- | --- |
| `2-1stm32` | 新建工程模板，只打开 GPIOC 时钟 | GPIO 时钟 |
| `3_1flash` | GPIO 推挽输出，LED 闪烁 | GPIO 输出 |
| `3-2 LED流水灯` | 三个 LED 流水灯 | GPIO 输出 |
| `3-3 蜂鸣器` | 蜂鸣器发声控制 | GPIO 输出 |
| `3-4 按键控制LED` | 按键输入控制 LED | GPIO 输入 |
| `3_4` | 按键控制 LED（重复练习版） | GPIO 输入 |
| `4-1 OLED显示屏` | OLED 屏幕显示 | GPIO 模拟时序 |
| `4-1oled` | OLED 显示 + 计次功能 | GPIO 模拟时序 |
| `5-1 对射式红外传感器计次` | 红外对射传感器计次 | EXTI 外部中断 |
| `5-2 旋转编码器计次` | 旋转编码器计数 | EXTI 外部中断 |
| `5-3 按键计时器` | 按键计时，定时器中断 | TIM 定时器中断 |
| `6-1 定时器定时中断` | 定时器定时中断，OLED 显示秒数 | TIM 内部时钟 |
| `6-1定时器外部时钟` | 红外对射接 PA0，外部时钟模式计次 | TIM 外部时钟 ETR |

> `3_4` 与 `3-4 按键控制LED`、`4-1oled` 与 `4-1 OLED显示屏` 内容相近，是同一节课的两次练习，保留下来作为对比。

## 编译方式

1. 用 Keil MDK 打开对应目录下的 `Project.uvprojx`
2. F7 编译，ST-Link 下载
3. 各工程 `Objects/` 目录为编译产物，已加入 `.gitignore`，不会提交

## 约定

- 每完成一个外设练习提交一次，提交信息格式：`feat(外设): 做了什么`
- 调通一个阶段就打一个 tag，例如 `v0.3-gpio`、`v0.5-exti`、`v0.6-tim`
- 换电脑后先跑一次 `keilkill.bat` 清掉旧的编译缓存，再重新编译

## 后续计划

- [ ] ADC 单通道采样 + DMA
- [ ] USART 串口通信与重定向 printf
- [ ] I2C 读写传感器（如 MPU6050 / AHT20）
- [ ] FreeRTOS 任务调度
- [ ] 综合项目：环境数据采集与上报

## 参考

- [STM32F103 参考手册 RM0008](https://www.st.com/resource/en/reference_manual/cd00171190.pdf)
- [STM32F103x8/xB 数据手册](https://www.st.com/resource/en/datasheet/cd00161566.pdf)
