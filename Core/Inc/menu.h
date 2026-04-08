#ifndef __MENU_H
#define __MENU_H

#include "main.h"

void Peripheral_Init(void);
void Show_Clock_UI(void);
int First_Page_Clock(void);
int SettingPage(void);
int Menu(void);
int MPU6050_App(void); // 稍微改了个名字，防止和底层驱动重名
int Gradienter(void);

#endif
