/* Menu module - kept minimal as all UI logic is in freertos.c */
#include "menu.h"
#include "OLED.h"

void Peripheral_Init(void)
{
  /* Peripherals initialized in main.c */
}

void Show_Clock_UI(void)
{
  /* Display handled by DisplayTask in freertos.c */
}

int First_Page_Clock(void)
{
  return 0;
}

int SettingPage(void)
{
  return 0;
}

int Menu(void)
{
  return 0;
}

int MPU6050_App(void)
{
  return 0;
}

int Gradienter(void)
{
  return 0;
}
