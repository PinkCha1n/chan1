/* Key driver module */
#include "Key.h"

/* Global key state */
uint8_t Key_Num = 0;

void Key_Init(void)
{
  /* GPIO initialized in MX_GPIO_Init */
}

/**
  * Get last pressed key
  * 1: Left (PB1), 2: Right (PA6), 3: Confirm (PA4), 0: None
  */
uint8_t Key_GetNum(void)
{
  uint8_t Temp;
  if(Key_Num)
  {
    Temp = Key_Num;
    Key_Num = 0;
    return Temp;
  }
  return 0;
}

/**
  * Direct key state read for RTOS
  * Returns: 1: Left (PB1), 2: Right (PA6), 3: Confirm (PA4), 0: None
  */
uint8_t Key_GetNum_RTOS(void)
{
  if(HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
    return 1;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET)
    return 2;
  if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
    return 3;
  return 0;
}

/**
  * Get current button state
  */
uint8_t Key_GetState(void)
{
  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_RESET)
    return 1;
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET)
    return 2;
  if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET)
    return 3;
  return 0;
}

/**
  * Key scan for debouncing (called in task)
  */
void Key_Tick(void)
{
  static uint8_t count = 0;
  static uint8_t last_state = 0, current_state = 0;
  
  count++;
  if(count >= 20)
  {
    count = 0;
    last_state = current_state;
    current_state = Key_GetState();
    
    if(last_state != 0 && current_state == 0)
    {
      Key_Num = last_state;
    }
  }
}
