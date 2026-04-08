/* Simple test to verify system is working */
#include "main.h"
#include "OLED.h"
#include <stdio.h>

int main_test(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_RTC_Init();
  
  /* Test LED - should blink */
  uint32_t count = 0;
  
  while(1)
  {
    count++;
    
    /* Blink LED every 500ms to show MCU is running */
    if((count % 500) < 250)
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);  /* LED ON (low) */
    else
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);    /* LED OFF (high) */
    
    HAL_Delay(1);
  }
  
  return 0;
}
