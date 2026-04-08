#ifndef __KEY_H
#define __KEY_H

#include "main.h" // ���� HAL ������е�Ӳ���궨��

void Key_Init(void);
uint8_t Key_GetNum(void);
uint8_t Key_GetNum_RTOS(void);
uint8_t Key_GetState(void); // ��������Ҳ��¶���������������ļ�����
void Key_Tick(void);

#endif
