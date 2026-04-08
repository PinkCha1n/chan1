#include "LED.h"

/**
  * @brief  LED 初始化
  * @note   由于使用了 STM32CubeMX，PB15 的时钟开启和推挽输出模式配置
  * 已经在 main.c 的 MX_GPIO_Init() 中自动生成了。
  * 为了兼容你原有逻辑不报错，保留这个空函数。
  */
void LED_Init(void)
{
	// 留空，底层配置由 HAL 库接管
	// 你可以在这里加一句默认熄灭，确保上电状态稳定
	LED_OFF(); 
}

/**
  * @brief  点亮 LED
  * @note   根据你的硬件，PB15 低电平点亮
  */
void LED_ON(void)
{
	// 替换原来的 GPIO_ResetBits
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
}

/**
  * @brief  熄灭 LED
  * @note   根据你的硬件，PB15 高电平熄灭
  */
void LED_OFF(void)
{
	// 替换原来的 GPIO_SetBits
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
}

/**
  * @brief  翻转 LED 状态 (亮变灭，灭变亮)
  */
void LED_Turn(void)
{
	// HAL 库自带电平翻转函数，完美替代原来的 if-else 判断！
	// 同时修复了原来代码里误写成 GPIO_Pin_1 的 bug
	HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_15);
}
