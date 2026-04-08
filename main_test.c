/* Minimal LED test - just blink LED and show text on OLED */
#include "main.h"
#include "OLED.h"
#include <stdio.h>

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_RTC_Init();
  
  /* Initialize OLED */
  OLED_Init();
  OLED_Clear();
  
  /* Blink LED to confirm MCU is running */
  uint32_t tick_count = 0;
  
  while(1)
  {
    tick_count++;
    
    /* Blink LED every 500ms */
    if((tick_count % 500) < 250)
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);  /* LED ON */
    else
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);    /* LED OFF */
    
    /* Update OLED display */
    if((tick_count % 100) == 0)
    {
      OLED_Clear();
      OLED_ShowString(0, 0, "Smart Watch", 8);
      OLED_ShowString(0, 12, "v1.0", 8);
      OLED_ShowString(0, 24, "Running...", 8);
      
      char str[20];
      sprintf(str, "Tick: %ld", tick_count/100);
      OLED_ShowString(0, 36, str, 8);
      
      /* Check buttons */
      OLED_ShowString(0, 48, "BTN:", 8);
      if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
        OLED_ShowString(30, 48, "L", 8);
      if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET)
        OLED_ShowString(38, 48, "R", 8);
      if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
        OLED_ShowString(46, 48, "OK", 8);
      
      OLED_Update();
    }
    
    HAL_Delay(1);
  }
  
  return 0;
}
