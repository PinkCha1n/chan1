/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "main.h"
#include "OLED.h"
#include "SetTime.h"
#include "Key.h"
#include "MPU6050.h"
#include "rtc.h"
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
osThreadId_t defaultTaskHandle;
osThreadId_t displayTaskHandle;
osThreadId_t buttonTaskHandle;
osThreadId_t systemTaskHandle;

const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
const osThreadAttr_t displayTask_attributes = {
  .name = "displayTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
const osThreadAttr_t buttonTask_attributes = {
  .name = "buttonTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
const osThreadAttr_t systemTask_attributes = {
  .name = "systemTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
void StartDefaultTask(void *argument);

/* USER CODE BEGIN FunctionPrototypes */
void StartDisplayTask(void *argument);
void StartButtonTask(void *argument);
void StartSystemTask(void *argument);
/* External variables provided by other modules */
extern RTC_HandleTypeDef hrtc;
extern uint8_t stopwatch_running;
extern uint8_t stopwatch_minutes;
extern uint8_t stopwatch_seconds;
extern uint16_t stopwatch_ms;
extern uint8_t led_on;
extern volatile uint8_t menu_state;
extern volatile uint8_t menu_select;
extern volatile uint8_t func_menu_index;
extern volatile uint8_t set_menu_index;
extern volatile uint8_t func_active_index;
extern volatile uint8_t set_edit_mode;
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* creation of display, button and system tasks */
  displayTaskHandle = osThreadNew(StartDisplayTask, NULL, &displayTask_attributes);
  buttonTaskHandle = osThreadNew(StartButtonTask, NULL, &buttonTask_attributes);
  systemTaskHandle = osThreadNew(StartSystemTask, NULL, &systemTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/* Flag set when MPU6050 has been initialized by system task */
static volatile uint8_t mpu_ready = 0;
/* Shared MPU values updated by system task (avoids I2C in display task) */
static volatile int16_t mpu_ax = 0;
static volatile int16_t mpu_ay = 0;
static volatile int16_t mpu_az = 0;
static volatile int16_t mpu_gx = 0;
static volatile int16_t mpu_gy = 0;
static volatile int16_t mpu_gz = 0;
/* Request a one-time full clear on next display loop when entering an app */
static volatile uint8_t need_full_clear = 0;

/* Local edit buffer used when editing time fields */
static uint8_t edit_time_buf[6]; /* Year(0-99), Mon, Day, Hour, Min, Sec */

static void LoadRTCToBuf(uint8_t buf[6])
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

  buf[0] = sDate.Year;
  buf[1] = sDate.Month;
  buf[2] = sDate.Date;
  buf[3] = sTime.Hours;
  buf[4] = sTime.Minutes;
  buf[5] = sTime.Seconds;
}

static void SaveBufToRTC(uint8_t buf[6])
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  sDate.Year = buf[0];
  sDate.Month = buf[1];
  sDate.Date = buf[2];

  sTime.Hours = buf[3];
  sTime.Minutes = buf[4];
  sTime.Seconds = buf[5];

  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

/* Display task: renders clock, function menu, and settings UI */
void StartDisplayTask(void *argument)
{
  char display_str[64];
  /* Initialize OLED once (I2C1 must be initialized in main before scheduler start) */
  OLED_Init();
  static int last_menu_state_display = -1;
  static int last_func_active_display = -1;
  for(;;)
  {
    /* If requested, perform a one-time full clear before drawing new app */
    if (need_full_clear)
    {
      OLED_Clear();
      OLED_Update();
      need_full_clear = 0;
      osDelay(20);
    }
    /* Fast-path: when in stopwatch active view, do partial updates at higher rate */
    if(menu_state == 3 && func_active_index == 3)
    {
      static int last_menu_state = -1;
      static int last_func_active = -1;
      static uint8_t last_running = 0xFF;
      static uint8_t last_min = 0xFF, last_sec = 0xFF;
      static uint16_t last_ms = 0xFFFF;

      /* First entry into this view: full redraw */
      if(last_menu_state != menu_state || last_func_active != func_active_index)
      {
        OLED_Clear();
        OLED_ShowString(30, 0, "Stopwatch", 8);
        char buf[32];
        sprintf(buf, "%02d:%02d.%03d", stopwatch_minutes, stopwatch_seconds, stopwatch_ms);
        OLED_ShowString(20, 22, buf, 12);
        OLED_ShowString(0, 56, "< Back", 6);
        OLED_ShowString(64, 56, "Reset", 6);
        if(stopwatch_running)
          OLED_ShowString(100, 56, "Stop", 6);
        else
          OLED_ShowString(100, 56, "Start", 6);
        OLED_Update();

        last_running = stopwatch_running;
        last_min = stopwatch_minutes;
        last_sec = stopwatch_seconds;
        last_ms = stopwatch_ms;
        last_menu_state = menu_state;
        last_func_active = func_active_index;
      }
      else
      {
        /* Update only changing parts: digits and start/stop label */
        if(last_min != stopwatch_minutes || last_sec != stopwatch_seconds || last_ms != stopwatch_ms)
        {
          char buf[32];
          sprintf(buf, "%02d:%02d.%03d", stopwatch_minutes, stopwatch_seconds, stopwatch_ms);
          OLED_ShowString(20, 22, buf, 12);
          /* update only digit area */
          OLED_UpdateArea(20, 20, 108, 24);
          last_min = stopwatch_minutes;
          last_sec = stopwatch_seconds;
          last_ms = stopwatch_ms;
        }
        if(last_running != stopwatch_running)
        {
          if(stopwatch_running)
            OLED_ShowString(100, 56, "Stop", 6);
          else
            OLED_ShowString(100, 56, "Start", 6);
          OLED_UpdateArea(100, 56, 28, 8);
          last_running = stopwatch_running;
        }
      }

      osDelay(30); /* ~33Hz partial refresh for smoother ms */
      continue; /* skip the full redraw path */
    }

    /* Avoid clearing when in Level view (we do area-updates there) */
    if (!(menu_state == 3 && func_active_index == 1))
    {
      OLED_Clear();
    }

    if(menu_state == 0)
    {
      /* Main clock view */
      RTC_TimeTypeDef sTime = {0};
      RTC_DateTypeDef sDate = {0};
      HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
      HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

      /* Date top-left */
      sprintf(display_str, "%04d-%02d-%02d", 2000 + sDate.Year, sDate.Month, sDate.Date);
      OLED_ShowString(0, 0, display_str, 6);

      /* Time center (hours:minutes) */
      sprintf(display_str, "%02d:%02d", sTime.Hours, sTime.Minutes);
      OLED_ShowString(16, 16, display_str, 12);
      /* Seconds small */
      sprintf(display_str, ":%02d", sTime.Seconds);
      OLED_ShowString(96, 24, display_str, 6);

      /* Bottom selection markers */
      if(menu_select == 0)
        OLED_ReverseArea(0, 56, 64, 8);
      else
        OLED_ReverseArea(64, 56, 64, 8);

      OLED_ShowString(2, 56, "Menu", 6);
      OLED_ShowString(70, 56, "Settings", 6);
    }
    else if(menu_state == 1)
    {
      /* Function menu */
      OLED_ShowString(30, 0, "Functions", 8);
      char *func_names[] = {"Flashlight", "Level", "Yaw", "Stopwatch"};
      for(int i = 0; i < 4; i++)
      {
        uint8_t y = 14 + i * 11;
        if(i == func_menu_index)
          OLED_ReverseArea(0, y - 2, 128, 10);
        OLED_ShowString(5, y, func_names[i], 6);
      }
      OLED_ShowString(0, 56, "< Back", 6);
      OLED_ShowString(90, 56, "Enter", 6);
    }
    else if(menu_state == 3)
    {
      /* Active function screen */
      if(func_active_index == 0) /* Flashlight */
      {
        OLED_ShowString(30, 0, "Flashlight", 8);
        if(led_on)
          OLED_ShowString(40, 28, "ON", 12);
        else
          OLED_ShowString(36, 28, "OFF", 12);
        OLED_ShowString(0, 56, "< Back", 6);
      }
      else if(func_active_index == 1) /* Level */
      {
        /* If MPU not initialized yet, show hint and skip sensor read */
        if (!mpu_ready)
        {
          OLED_ShowString(34, 20, "MPU init...", 8);
          OLED_ShowString(0, 56, "< Back", 6);
          OLED_UpdateArea(34, 20, 60, 12);
          OLED_UpdateArea(0, 56, 128, 8);
          osDelay(200);
          continue;
        }
        else
        {
          /* On first entry to Level view, clear whole screen once to avoid leftover pixels */
          if (last_menu_state_display != menu_state || last_func_active_display != func_active_index)
          {
            OLED_Clear();
            OLED_Update();
            last_menu_state_display = menu_state;
            last_func_active_display = func_active_index;
          }
          /* Use shared MPU samples (updated in system task) */
          float angleXf = atan2f((float)mpu_ax, (float)mpu_az) * 57.2957795f; /* X-axis tilt */
          float angleYf = atan2f((float)mpu_ay, (float)mpu_az) * 57.2957795f; /* Y-axis tilt */

          /* Screen and circle geometry */
          const int16_t screenW = 128;
          const int16_t screenH = 64;
          const int16_t cx = screenW / 2;
          const int16_t cy = screenH / 2;
          const int16_t radius = ( (screenW < screenH) ? (screenW/2) : (screenH/2) ) - 2;

          /* Map tilt to circle displacement. maxAngle maps to full radius. */
          const float maxAngle = 45.0f; /* degrees mapped to edge */
          float dx = -(angleYf / maxAngle) * (float)radius;
          float dy = -(angleXf / maxAngle) * (float)radius;
          if (dx > radius) dx = radius; if (dx < -radius) dx = -radius;
          if (dy > radius) dy = radius; if (dy < -radius) dy = -radius;

          int16_t dotX = cx + (int16_t)dx;
          int16_t dotY = cy + (int16_t)dy;

          /* Bounding box for circle area */
          int16_t x0 = cx - radius;
          int16_t y0 = cy - radius;
          if (x0 < 0) x0 = 0;
          if (y0 < 0) y0 = 0;
          uint8_t width = (uint8_t)((cx + radius) - x0 + 1);
          uint8_t height = (uint8_t)((cy + radius) - y0 + 1);

          /* Clear and redraw only the circle area in the buffer, then transmit it */
          OLED_ClearArea(x0, y0, width, height);
          OLED_DrawCircle(cx, cy, radius, OLED_UNFILLED);
          OLED_DrawLine(cx - radius, cy, cx + radius, cy);
          OLED_DrawLine(cx, cy - radius, cx, cy + radius);
          OLED_DrawCircle(dotX, dotY, 2, OLED_FILLED);
          OLED_UpdateArea(x0, y0, width, height);

          /* Update numeric readout (top) and back label (bottom) separately */
          char buf[32];
          sprintf(buf, "X:%+03d Y:%+03d", (int)angleXf, (int)angleYf);
          OLED_ShowString(0, 0, buf, 6);
          OLED_UpdateArea(0, 0, 128, 8);
          OLED_ShowString(0, 56, "< Back", 6);
          OLED_UpdateArea(0, 56, 128, 8);

          osDelay(30); /* ~33Hz updates for smoother movement */
          continue;
        }
      }
      else if(func_active_index == 2) /* Yaw */
      {
        /* Use shared gyro Z value updated by system task */
        char buf[32];
        int yaw10 = (int)(((float)mpu_gz / 131.0f) * 10.0f);
        OLED_ShowString(36, 0, "Yaw", 8);
        sprintf(buf, "Rate:%+d.%d dps", yaw10/10, abs(yaw10)%10);
        OLED_ShowString(0, 24, buf, 8);
        OLED_ShowString(0, 56, "< Back", 6);
      }
      else if(func_active_index == 3) /* Stopwatch */
      {
        char buf[32];
        OLED_ShowString(30, 0, "Stopwatch", 8);
        sprintf(buf, "%02d:%02d.%03d", stopwatch_minutes, stopwatch_seconds, stopwatch_ms);
        OLED_ShowString(20, 22, buf, 12);
        OLED_ShowString(0, 56, "< Back", 6);
        OLED_ShowString(90, 56, "Start", 6);
      }
    }
    else if(menu_state == 2)
    {
      if(set_edit_mode == 0)
      {
        OLED_ShowString(30, 0, "Set Time", 8);
        char *set_names[] = {"Year", "Mon", "Day", "Hour", "Min", "Sec"};
        LoadRTCToBuf(edit_time_buf);
        for(int i = 0; i < 6; i++)
        {
          uint8_t y = 14 + i * 8;
          sprintf(display_str, "%s: %02d", set_names[i], edit_time_buf[i]);
          if(i == set_menu_index)
            OLED_ReverseArea(0, y - 2, 128, 10);
          OLED_ShowString(0, y, display_str, 6);
        }
        OLED_ShowString(0, 56, "< Back", 6);
        OLED_ShowString(90, 56, "Edit", 6);
      }
      else
      {
        /* Editing a single field */
        char *set_names[] = {"Year", "Mon", "Day", "Hour", "Min", "Sec"};
        char hint_buf[32];
        sprintf(display_str, "%s", set_names[set_menu_index]);
        OLED_ShowString(24, 0, "Edit Value", 8);
        sprintf(display_str, "%02d", edit_time_buf[set_menu_index]);
        OLED_ShowString(36, 20, display_str, 12);
        /* Hint */
        switch(set_menu_index)
        {
          case 0: sprintf(hint_buf, "Year: 00-99"); break;
          case 1: sprintf(hint_buf, "Month: 1-12"); break;
          case 2: sprintf(hint_buf, "Day: 1-31"); break;
          case 3: sprintf(hint_buf, "Hour: 0-23"); break;
          default: sprintf(hint_buf, "00-59"); break;
        }
        OLED_ShowString(8, 44, hint_buf, 6);
        OLED_ShowString(0, 56, "< Cancel", 6);
        OLED_ShowString(90, 56, "OK", 6);
      }
    }

    OLED_Update();
    osDelay(100); /* increase non-stopwatch frame rate to 10Hz */
  }
}

/* Button task: handle left/right/confirm and editing actions */
void StartButtonTask(void *argument)
{
  GPIO_PinState left_last = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
  GPIO_PinState right_last = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
  GPIO_PinState confirm_last = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);

  for(;;)
  {
    GPIO_PinState left_read = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1);
    GPIO_PinState right_read = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6);
    GPIO_PinState confirm_read = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4);

    /* Left falling edge */
    if(left_read == GPIO_PIN_RESET && left_last == GPIO_PIN_SET)
    {
      if(menu_state == 0)
      {
        menu_select = 0;
      }
      else if(menu_state == 3)
      {
        /* From active function -> back to function menu */
        menu_state = 1;
      }
      else
      {
        /* Back to main screen (also save if leaving settings) */
        if(menu_state == 2) SaveBufToRTC(edit_time_buf);
        set_edit_mode = 0;
        menu_state = 0;
        menu_select = 0;
      }
    }

    /* Right falling edge */
    if(right_read == GPIO_PIN_RESET && right_last == GPIO_PIN_SET)
    {
      if(menu_state == 0)
      {
        menu_select = 1;
      }
      else if(menu_state == 1)
      {
        func_menu_index = (func_menu_index + 1) % 4;
      }
      else if(menu_state == 2)
      {
        if(set_edit_mode)
        {
          /* increase field */
          uint8_t idx = set_menu_index;
          switch(idx)
          {
            case 0: edit_time_buf[idx] = (edit_time_buf[idx] + 1) % 100; break;
            case 1: edit_time_buf[idx] = (edit_time_buf[idx] % 12) + 1; break;
            case 2: edit_time_buf[idx] = (edit_time_buf[idx] % 31) + 1; break;
            case 3: edit_time_buf[idx] = (edit_time_buf[idx] + 1) % 24; break;
            default: edit_time_buf[idx] = (edit_time_buf[idx] + 1) % 60; break;
          }
        }
        else
        {
          set_menu_index = (set_menu_index + 1) % 6;
        }
      }
      else if(menu_state == 3)
      {
        /* In active function screen: right button - for stopwatch, reset when stopped */
        if(func_active_index == 3)
        {
          if(!stopwatch_running)
          {
            stopwatch_ms = 0;
            stopwatch_seconds = 0;
            stopwatch_minutes = 0;
          }
        }
      }
    }

    /* Confirm falling edge */
    if(confirm_read == GPIO_PIN_RESET && confirm_last == GPIO_PIN_SET)
    {
      if(menu_state == 0)
      {
        if(menu_select == 0)
        {
          menu_state = 1;
          func_menu_index = 0;
        }
        else
        {
          /* request full clear before entering settings app */
          need_full_clear = 1;
          menu_state = 2;
          set_menu_index = 0;
          set_edit_mode = 0;
          LoadRTCToBuf(edit_time_buf);
        }
      }
      else if(menu_state == 1)
      {
        /* Enter selected function view */
        /* request full clear before entering function app */
        need_full_clear = 1;
        func_active_index = func_menu_index;
        menu_state = 3; /* active function screen */
      }
      else if(menu_state == 3)
      {
        /* In active function screen: confirm triggers actions */
        if(func_active_index == 0)
        {
          led_on = !led_on;
        }
        else if(func_active_index == 3)
        {
          stopwatch_running = !stopwatch_running;
        }
      }
      else if(menu_state == 2)
      {
        if(set_edit_mode == 0)
        {
          /* enter edit mode */
          set_edit_mode = 1;
        }
        else
        {
          /* confirm current field and move to next */
          if(set_menu_index < 5)
          {
            set_menu_index++;
            /* keep editing next field */
            set_edit_mode = 1;
          }
          else
          {
            /* last field -> save and return */
            SaveBufToRTC(edit_time_buf);
            set_edit_mode = 0;
            menu_state = 0;
            menu_select = 0;
          }
        }
      }
    }

    left_last = left_read;
    right_last = right_read;
    confirm_last = confirm_read;

    osDelay(60);
  }
}

/* System task: stopwatch timing and LED control */
void StartSystemTask(void *argument)
{
  /* initialize MPU6050 sensor (I2C2) */
  MPU6050_Init();
  /* mark MPU as ready for other tasks */
  mpu_ready = 1;
  osDelay(50);
  for(;;)
  {
    /* Read MPU data here once per loop to avoid I2C from display task */
    if (mpu_ready)
    {
      int16_t tax, tay, taz, tgx, tgy, tgz;
      MPU6050_GetData(&tax, &tay, &taz, &tgx, &tgy, &tgz);
      mpu_ax = tax;
      mpu_ay = tay;
      mpu_az = taz;
      mpu_gx = tgx;
      mpu_gy = tgy;
      mpu_gz = tgz;
    }

    if(stopwatch_running)
    {
      stopwatch_ms += 10; /* 10ms resolution */
      if(stopwatch_ms >= 1000)
      {
        stopwatch_ms -= 1000;
        stopwatch_seconds++;
        if(stopwatch_seconds >= 60)
        {
          stopwatch_seconds = 0;
          stopwatch_minutes++;
          if(stopwatch_minutes >= 100) stopwatch_minutes = 0;
        }
      }
    }

    /* LED: PB15 active low */
    if(led_on)
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_RESET);
    else
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, GPIO_PIN_SET);

    osDelay(10);
  }
}

/* USER CODE END Application */

