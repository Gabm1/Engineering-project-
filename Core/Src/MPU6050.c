/*
 * MPU6050.c
 *
 *  Created on: Apr 21, 2025
 *      Author: micha
 */
#include "MPU6050.h"
#include "stdbool.h"
//uint8_t init_MPU6050() {
//    uint8_t reg;
//    uint8_t id;
//
//    // 1. Check if MPU6050 responds to WHO_AM_I
//    reg = MPU6050_WHO_AM_I;
//    if (HAL_I2C_Master_Transmit(&hi2c1, MPU6050_I2CADDR_DEFAULT, &reg, 1, HAL_MAX_DELAY) != HAL_OK) {
//        return 0; // Transmit failed
//    }
//
//    if (HAL_I2C_Master_Receive(&hi2c1, MPU6050_I2CADDR_DEFAULT, &id, 1, HAL_MAX_DELAY) != HAL_OK) {
//        return 0; // Receive failed
//    }
//
//    if (id != MPU6050_DEVICE_ID) {
//        return 0; // Wrong device ID
//    }
//
//    // 2. Wake up the MPU6050 by writing 0x00 to PWR_MGMT_1
//    uint8_t power_on_data[2] = {MPU6050_PWR_MGMT_1, 0x00};
//    if (HAL_I2C_Master_Transmit(&hi2c1, MPU6050_I2CADDR_DEFAULT, power_on_data, 2, HAL_MAX_DELAY) != HAL_OK) {
//        return 0; // Failed to wake device
//    }
//
//    return 1; // Success!
//}
uint8_t init_MPU6050() {
	uint8_t id;
	uint8_t wakeup = 0x01;
	HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2CADDR_DEFAULT, MPU6050_WHO_AM_I, 1, &id, 1, HAL_MAX_DELAY);

	if(id != MPU6050_DEVICE_ID ){
		return 0;
	}
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_I2CADDR_DEFAULT, MPU6050_PWR_MGMT_1, 1, &wakeup, 1, HAL_MAX_DELAY);
	return 1;
}

void Gyro_range_set(mpu6050_gyro_range_t range){
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_I2CADDR_DEFAULT, MPU6050_GYRO_CONFIG, 1, &range, 1, HAL_MAX_DELAY);
}
void Acel_range_set(mpu6050_accel_range_t range){
	HAL_I2C_Mem_Write(&hi2c1, MPU6050_I2CADDR_DEFAULT, MPU6050_ACCEL_CONFIG, 1, &range, 1, HAL_MAX_DELAY);
}
void Read_accel(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t temp[6];

    HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2CADDR_DEFAULT, 0x3B, 1, temp, 6, HAL_MAX_DELAY);

    *x = (int16_t)(temp[0] << 8 | temp[1]);
    *y = (int16_t)(temp[2] << 8 | temp[3]);
    *z = (int16_t)(temp[4] << 8 | temp[5]);
}
void Read_gyro(int16_t *x, int16_t *y, int16_t *z) {
    uint8_t temp[6];

    HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2CADDR_DEFAULT, 0x43, 1, temp, 6, HAL_MAX_DELAY);

    *x = (int16_t)(temp[0] << 8 | temp[1]);
    *y = (int16_t)(temp[2] << 8 | temp[3]);
    *z = (int16_t)(temp[4] << 8 | temp[5]);
}
float Read_temperature(void) {
    uint8_t temp[2];
    HAL_I2C_Mem_Read(&hi2c1, MPU6050_I2CADDR_DEFAULT, 0x41, 1, temp, 2, HAL_MAX_DELAY);
    int16_t raw_temp = (int16_t)(temp[0] << 8 | temp[1]);
    return (raw_temp / 340.0) + 36.53;
}

