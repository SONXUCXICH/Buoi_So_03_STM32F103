#ifndef BMP280_H
#define BMP280_H

#include "stm32f10x.h"

#define BMP280_ADDR       0x76

#define BMP280_REG_ID     0xD0
#define BMP280_REG_CTRL   0xF4
#define BMP280_REG_CONFIG 0xF5

#define BMP280_REG_PRESS  0xF7
#define BMP280_REG_TEMP   0xFA

#define BMP280_CHIP_ID    0x58

void I2C1_Init(void);

uint8_t BMP280_Init(void);

uint8_t BMP280_ReadID(void);

void BMP280_ReadTemperature(float *temperature);

void BMP280_ReadPressure(float *pressure);

#endif