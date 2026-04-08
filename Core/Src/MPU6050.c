#include "MPU6050.h"
#include "MPU6050_Reg.h"
// 注意：移除了对 MyI2C.h 的包含，不需要它了！

// 引入由 CubeMX 生成的 I2C2 句柄 (PB10, PB11)
extern I2C_HandleTypeDef hi2c2;

// MPU6050 的 I2C 从机地址 (左移一位后的值)
#define MPU6050_ADDRESS		0xD0

/**
  * @brief  向 MPU6050 写入一个字节的数据
  * @note   使用 HAL 库硬件 I2C 阻塞写入，替代原有的软件模拟时序
  */
void MPU6050_WriteReg(uint8_t RegAddress, uint8_t Data)
{
	// 参数：I2C句柄, 设备地址, 寄存器地址, 寄存器地址长度(8位), 数据存放指针, 数据长度, 超时时间(ms)
	HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDRESS, RegAddress, I2C_MEMADD_SIZE_8BIT, &Data, 1, 100);
}

/**
  * @brief  从 MPU6050 读取一个字节的数据
  * @note   使用 HAL 库硬件 I2C 阻塞读取，替代原有的软件模拟时序
  */
uint8_t MPU6050_ReadReg(uint8_t RegAddress)
{
	uint8_t Data;
	// 参数与 Write 类似，只是把数据读出到 Data 变量中
	HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDRESS, RegAddress, I2C_MEMADD_SIZE_8BIT, &Data, 1, 100);
	return Data;
}

void MPU6050_Init(void)
{
	// MyI2C_Init(); // 这一句删掉！因为硬件 I2C2 已经在 main.c 的 MX_I2C2_Init() 里初始化过了
	
	MPU6050_WriteReg(MPU6050_PWR_MGMT_1, 0x01);
	MPU6050_WriteReg(MPU6050_PWR_MGMT_2, 0x00);
	MPU6050_WriteReg(MPU6050_SMPLRT_DIV, 0x04);
	MPU6050_WriteReg(MPU6050_CONFIG, 0x06);
	MPU6050_WriteReg(MPU6050_GYRO_CONFIG, 0x18);
	MPU6050_WriteReg(MPU6050_ACCEL_CONFIG, 0x18);
}

uint8_t MPU6050_GetID(void)
{
	return MPU6050_ReadReg(MPU6050_WHO_AM_I);
}

void MPU6050_GetData(int16_t *AccX, int16_t *AccY, int16_t *AccZ, 
						int16_t *GyroX, int16_t *GyroY, int16_t *GyroZ)
{
	uint8_t DataH, DataL;
	
	// 读取逻辑保持完全不变，底层的 ReadReg 已经接管了所有脏活累活
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_XOUT_L);
	*AccX = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_YOUT_L);
	*AccY = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_ACCEL_ZOUT_L);
	*AccZ = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_XOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_GYRO_XOUT_L);
	*GyroX = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_YOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_GYRO_YOUT_L);
	*GyroY = (DataH << 8) | DataL;
	
	DataH = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_H);
	DataL = MPU6050_ReadReg(MPU6050_GYRO_ZOUT_L);
	*GyroZ = (DataH << 8) | DataL;
}
