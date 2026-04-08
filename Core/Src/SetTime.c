#include "SetTime.h"
#include "OLED.h"
#include "Key.h"
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"
#include "rtc.h"
#include <stdio.h>

extern uint8_t Key_GetNum_RTOS(void);
extern const uint8_t Return[];
extern RTC_HandleTypeDef hrtc;

/* Global variables - ALL DEFINITIONS */
uint8_t KeyNum = 0;
uint8_t stopwatch_running = 0;
uint8_t stopwatch_minutes = 0;
uint8_t stopwatch_seconds = 0;
uint16_t stopwatch_ms = 0;
volatile uint8_t current_page = 0;  /* volatile for RTOS multi-task access */
uint8_t led_on = 0;

/* Menu system variables */
volatile uint8_t menu_state = 0;      /* 0: Main clock, 1: Function menu, 2: Settings menu */
volatile uint8_t menu_select = 0;     /* For main page: 0=Menu, 1=Settings */
volatile uint8_t func_menu_index = 0; /* 0=Flashlight, 1=Level, 2=Yaw, 3=Stopwatch */
volatile uint8_t set_menu_index = 0;  /* 0=Year, 1=Month, 2=Date, 3=Hour, 4=Minute, 5=Second */
volatile uint8_t set_edit_value = 0;  /* Reserved: not used directly */
volatile uint8_t func_active_index = 0; /* Active function when entered */
volatile uint8_t set_edit_mode = 0;   /* 0: not editing, 1: editing current field */

/* RTC time storage */
int8_t MyRTC_Time[6];

/**
  * Load RTC time into buffer
  */
void Load_RTC_Time(void)
{
	RTC_DateTypeDef sDate = {0};
	RTC_TimeTypeDef sTime = {0};
	
	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
	
	MyRTC_Time[0] = sDate.Year;
	MyRTC_Time[1] = sDate.Month;
	MyRTC_Time[2] = sDate.Date;
	MyRTC_Time[3] = sTime.Hours;
	MyRTC_Time[4] = sTime.Minutes;
	MyRTC_Time[5] = sTime.Seconds;
}

/**
  * Save RTC time from buffer
  */
void Save_RTC_Time(void)
{
	RTC_DateTypeDef sDate = {0};
	RTC_TimeTypeDef sTime = {0};
	
	sDate.Year = MyRTC_Time[0];
	sDate.Month = MyRTC_Time[1];
	sDate.Date = MyRTC_Time[2];
	sTime.Hours = MyRTC_Time[3];
	sTime.Minutes = MyRTC_Time[4];
	sTime.Seconds = MyRTC_Time[5];
	
	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

/**
  * Set time via OLED interface
  */
void Settime(void)
{
	Load_RTC_Time();
	
	int8_t set_index = 0;  /* 0-5: Year, Month, Date, Hour, Minute, Second */
	char time_str[20];
	
	while(1)
	{
		OLED_Clear();
		OLED_ShowString(0, 0, "Set Time/Date", 8);
		
		/* Display current values */
		sprintf(time_str, "Y:%02d M:%02d D:%02d", MyRTC_Time[0], MyRTC_Time[1], MyRTC_Time[2]);
		OLED_ShowString(0, 16, time_str, 6);
		
		sprintf(time_str, "H:%02d M:%02d S:%02d", MyRTC_Time[3], MyRTC_Time[4], MyRTC_Time[5]);
		OLED_ShowString(0, 28, time_str, 6);
		
		/* Highlight current editing field */
		char *field_names[] = {"Year", "Month", "Date", "Hour", "Minute", "Second"};
		OLED_ShowString(0, 40, field_names[set_index], 8);
		sprintf(time_str, "Val:%02d", MyRTC_Time[set_index]);
		OLED_ShowString(0, 48, time_str, 8);
		
		OLED_ShowString(0, 56, "L/R:+/- OK:Next ESC:Save", 6);
		OLED_Update();
		
		KeyNum = Key_GetNum_RTOS();
		
		if(KeyNum == 1)  /* Left - decrease */
		{
			MyRTC_Time[set_index]--;
			osDelay(200);
		}
		else if(KeyNum == 2)  /* Right - increase */
		{
			MyRTC_Time[set_index]++;
			osDelay(200);
		}
		else if(KeyNum == 3)  /* Confirm - next field */
		{
			set_index++;
			if(set_index >= 6)
			{
				Save_RTC_Time();
				break;
			}
			osDelay(200);
		}
		
		osDelay(100);
	}
}
