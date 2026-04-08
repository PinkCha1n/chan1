#include "MyI2C.h"

/**
  * @brief  微秒级短延时 (软件 I2C 专用)
  * @note   在 FreeRTOS 中无法使用精准的 us 级系统延时，
  * 考虑到 72MHz 的主频，使用简单的 NOP 循环进行大约 10us 的粗略延时。
  * I2C 是同步通信，稍微被 RTOS 打断拉长时钟也无妨，不会导致时序崩溃。
  */
static void I2C_Delay(void)
{
    uint32_t i = 150; 
    while(i--) 
    {
        __NOP(); // 空指令，防止被编译器过度优化
    }
}

void MyI2C_W_SCL(uint8_t BitValue)
{
    // 替换为 HAL 库函数
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);
    I2C_Delay();
}

void MyI2C_W_SDA(uint8_t BitValue)
{
    // 替换为 HAL 库函数
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, BitValue ? GPIO_PIN_SET : GPIO_PIN_RESET);
    I2C_Delay();
}

uint8_t MyI2C_R_SDA(void)
{
    uint8_t BitValue;
    // 替换为 HAL 库读取函数
    BitValue = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) ? 1 : 0;
    I2C_Delay();
    return BitValue;
}

void MyI2C_Init(void)
{
    // 底层初始化已被 CubeMX 的 MX_GPIO_Init() 接管
    // ?? 前提警告：请务必确保在 CubeMX 中把 PB10 和 PB11 设置为：
    // GPIO output level: High
    // GPIO mode: Output Open Drain (开漏输出，软件I2C必须用开漏)
    // GPIO Pull-up/Pull-down: Pull-up (上拉)
    
    // 初始化时将总线拉高释放
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_10, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_11, GPIO_PIN_SET);
}

void MyI2C_Start(void)
{
    MyI2C_W_SDA(1);
    MyI2C_W_SCL(1);
    MyI2C_W_SDA(0);
    MyI2C_W_SCL(0);
}

void MyI2C_Stop(void)
{
    MyI2C_W_SDA(0);
    MyI2C_W_SCL(1);
    MyI2C_W_SDA(1);
}

void MyI2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        MyI2C_W_SDA(!!(Byte & (0x80 >> i)));
        MyI2C_W_SCL(1);
        MyI2C_W_SCL(0);
    }
}

uint8_t MyI2C_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;
    MyI2C_W_SDA(1); // 释放 SDA，准备读取
    for (i = 0; i < 8; i++)
    {
        MyI2C_W_SCL(1);
        if (MyI2C_R_SDA()) { Byte |= (0x80 >> i); }
        MyI2C_W_SCL(0);
    }
    return Byte;
}

void MyI2C_SendAck(uint8_t AckBit)
{
    MyI2C_W_SDA(AckBit);
    MyI2C_W_SCL(1);
    MyI2C_W_SCL(0);
}

uint8_t MyI2C_ReceiveAck(void)
{
    uint8_t AckBit;
    MyI2C_W_SDA(1); // 释放 SDA，准备读取应答
    MyI2C_W_SCL(1);
    AckBit = MyI2C_R_SDA();
    MyI2C_W_SCL(0);
    return AckBit;
}
