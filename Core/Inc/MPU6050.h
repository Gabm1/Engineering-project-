/*
 * MPU6050.h
 *
 *  Created on: Apr 21, 2025
 *      Author: micha
 */

#ifndef INC_MPU6050_H_
#define INC_MPU6050_H_
#include "main.h"
extern I2C_HandleTypeDef hi2c1;
// -----------------------------------------------------------------------------
// I2C Address and Device ID
// -----------------------------------------------------------------------------
///< Default I2C address (AD0 high)
#define MPU6050_I2CADDR_DEFAULT   (0x68 << 1)
#define MPU6050_DEVICE_ID         0x68  ///< Expected WHO_AM_I value

// -----------------------------------------------------------------------------
// Register Map
// -----------------------------------------------------------------------------

// Configuration Registers
#define MPU6050_SMPLRT_DIV        0x19  ///< Sample Rate Divider
#define MPU6050_CONFIG            0x1A  ///< General Configuration
#define MPU6050_GYRO_CONFIG       0x1B  ///< Gyroscope Configuration
#define MPU6050_ACCEL_CONFIG      0x1C  ///< Accelerometer Configuration

// Self-Test Registers
#define MPU6050_SELF_TEST_X       0x0D  ///< Self Test - X Axis
#define MPU6050_SELF_TEST_Y       0x0E  ///< Self Test - Y Axis
#define MPU6050_SELF_TEST_Z       0x0F  ///< Self Test - Z Axis
#define MPU6050_SELF_TEST_A       0x10  ///< Self Test - Accelerometer

// Interrupt and Motion Detection
#define MPU6050_INT_PIN_CONFIG    0x37  ///< Interrupt Pin Configuration
#define MPU6050_INT_ENABLE        0x38  ///< Interrupt Enable
#define MPU6050_INT_STATUS        0x3A  ///< Interrupt Status
#define MPU6050_MOT_THR           0x1F  ///< Motion Detection Threshold
#define MPU6050_MOT_DUR           0x20  ///< Motion Detection Duration

// Power Management
#define MPU6050_PWR_MGMT_1        0x6B  ///< Power Management 1
#define MPU6050_PWR_MGMT_2        0x6C  ///< Power Management 2

// User Control & Reset
#define MPU6050_USER_CTRL         0x6A  ///< User Control
#define MPU6050_SIGNAL_PATH_RESET 0x68  ///< Signal Path Reset

// Sensor Output
#define MPU6050_ACCEL_OUT         0x3B  ///< Starting Address for Sensor Data
#define MPU6050_TEMP_H            0x41  ///< Temperature High Byte
#define MPU6050_TEMP_L            0x42  ///< Temperature Low Byte

// Identification
#define MPU6050_WHO_AM_I          0x75  ///< Device ID Register
/**
 * Allowed values for `setFsyncSampleOutput`.
 */
typedef enum fsync_out {
  MPU6050_FSYNC_OUT_DISABLED,
  MPU6050_FSYNC_OUT_TEMP,
  MPU6050_FSYNC_OUT_GYROX,
  MPU6050_FSYNC_OUT_GYROY,
  MPU6050_FSYNC_OUT_GYROZ,
  MPU6050_FSYNC_OUT_ACCELX,
  MPU6050_FSYNC_OUT_ACCELY,
  MPU6050_FSYNC_OUT_ACCEL_Z,
} mpu6050_fsync_out_t;

/**
 * @brief Clock source options
 *
 * Allowed values for `setClock`.
 */
typedef enum clock_select {
  MPU6050_INTR_8MHz,
  MPU6050_PLL_GYROX,
  MPU6050_PLL_GYROY,
  MPU6050_PLL_GYROZ,
  MPU6050_PLL_EXT_32K,
  MPU6050_PLL_EXT_19MHz,
  MPU6050_STOP = 7,
} mpu6050_clock_select_t;

/**
 * @brief Accelerometer range options
 *
 * Allowed values for `setAccelerometerRange`.
 */
typedef enum {
  MPU6050_RANGE_2_G = 0b00,  ///< +/- 2g (default value)
  MPU6050_RANGE_4_G = 0b01,  ///< +/- 4g
  MPU6050_RANGE_8_G = 0b10,  ///< +/- 8g
  MPU6050_RANGE_16_G = 0b11, ///< +/- 16g
} mpu6050_accel_range_t;

/**
 * @brief Gyroscope range options
 *
 * Allowed values for `setGyroRange`.
 */
typedef enum {
  MPU6050_RANGE_250_DEG,  ///< +/- 250 deg/s (default value)
  MPU6050_RANGE_500_DEG,  ///< +/- 500 deg/s
  MPU6050_RANGE_1000_DEG, ///< +/- 1000 deg/s
  MPU6050_RANGE_2000_DEG, ///< +/- 2000 deg/s
} mpu6050_gyro_range_t;

/**
 * @brief Digital low pass filter bandthwidth options
 *
 * Allowed values for `setFilterBandwidth`.
 */
typedef enum {
  MPU6050_BAND_260_HZ, ///< Docs imply this disables the filter
  MPU6050_BAND_184_HZ, ///< 184 Hz
  MPU6050_BAND_94_HZ,  ///< 94 Hz
  MPU6050_BAND_44_HZ,  ///< 44 Hz
  MPU6050_BAND_21_HZ,  ///< 21 Hz
  MPU6050_BAND_10_HZ,  ///< 10 Hz
  MPU6050_BAND_5_HZ,   ///< 5 Hz
} mpu6050_bandwidth_t;

/**
 * @brief Accelerometer high pass filter options
 *
 * Allowed values for `setHighPassFilter`.
 */
typedef enum {
  MPU6050_HIGHPASS_DISABLE,
  MPU6050_HIGHPASS_5_HZ,
  MPU6050_HIGHPASS_2_5_HZ,
  MPU6050_HIGHPASS_1_25_HZ,
  MPU6050_HIGHPASS_0_63_HZ,
  MPU6050_HIGHPASS_UNUSED,
  MPU6050_HIGHPASS_HOLD,
} mpu6050_highpass_t;

/**
 * @brief Periodic measurement options
 *
 * Allowed values for `setCycleRate`.
 */
typedef enum {
  MPU6050_CYCLE_1_25_HZ, ///< 1.25 Hz
  MPU6050_CYCLE_5_HZ,    ///< 5 Hz
  MPU6050_CYCLE_20_HZ,   ///< 20 Hz
  MPU6050_CYCLE_40_HZ,   ///< 40 Hz
} mpu6050_cycle_rate_t;


uint8_t init_MPU6050();
void Gyro_range_set(mpu6050_gyro_range_t range);
void Acel_range_set(mpu6050_accel_range_t range);
void Read_accel(int16_t *x, int16_t *y, int16_t *z);
void Read_gyro(int16_t *x, int16_t *y, int16_t *z);
float Read_temperature(void);

#endif /* INC_MPU6050_H_ */
