/*
 * Simple OLED Test - No FreeRTOS, just direct calls
 * 用于诊断OLED和LED硬件是否正常工作
 */
#include "main.h"
#include "OLED.h"
#include "gpio.h"
#include <stdio.h>

void test_main(void)
{
  /* Initialize hardware */
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();  
  MX_RTC_Init();
  
  /* Long delay to wait for OLED power */
  for(int i = 0; i < 20; i++)
    HAL_Delay(50);
  
  /* Initialize OLED */
  OLED_Init();
  
  /* Simple test loop */
  uint32_t tick = 0;
  
  while(1)
  {
    tick++;
    
    /* Blink LED to confirm MCU is alive */
    if((tick % 500) < 250)
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);  /* LED ON */
    else
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);    /* LED OFF */
    
    /* Update OLED every 500ms */
    if((tick % 500) == 0)
    {
      OLED_Clear();
      
      OLED_ShowString(0, 0, "OLED Test", 8);
      OLED_ShowString(0, 16, "LED Blinking", 8);
      
      char tick_str[20];
      sprintf(tick_str, "Tick:%ld", tick/500);
      OLED_ShowString(0, 32, tick_str, 8);
      
      OLED_ShowString(0, 48, "Working!", 8);
      
      OLED_Update();
    }
    
    HAL_Delay(1);
  }
}
