/**
 * Minimal test - No FreeRTOS, only basic LED blink
 * This is the absolute minimum to test if MCU boots
 */

#include "stm32f1xx_hal.h"
#include "gpio.h"

void SystemClock_Config(void);

int main_minimal(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick */
  HAL_Init();
  
  /* Configure the system clock */
  SystemClock_Config();
  
  /* Initialize GPIO */
  MX_GPIO_Init();
  
  /* Simple blink loop */
  while(1)
  {
    /* LED ON (PB15 low) */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_Delay(500);
    
    /* LED OFF (PB15 high) */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);
    HAL_Delay(500);
  }
  
  return 0;
}
