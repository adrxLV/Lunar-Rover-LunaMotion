/*
 * =============================================================================
 * BMI323 6-Axis IMU Library for Arduino
 * Pure C Implementation with Comprehensive Documentation
 * =============================================================================
 * 
 * FILE: bmi323.h
 * 
 * DESCRIPTION:
 * This library provides a complete interface for the BMI323 6-axis IMU sensor
 * (3-axis accelerometer + 3-axis gyroscope + temperature sensor).
 * Adapted from STM32 HAL implementation to work with Arduino Wire library.
 * 
 * FEATURES:
 * - Auto I2C address detection (0x68 or 0x69)
 * - Raw and converted data access
 * - Non-blocking continuous reading
 * - Built-in sensor configuration
 * - Temperature measurement
 * - Comprehensive error handling
 * 
 * HARDWARE REQUIREMENTS:
 * - Arduino board with I2C support
 * - BMI323 sensor module
 * - 3.3V power supply (IMPORTANT: Do NOT use 5V!)
 * - Pull-up resistors on SDA/SCL (usually built-in on modules)
 * 
 * WIRING:
 * BMI323    Arduino Uno/Nano    Arduino Mega    ESP32
 * ------    ----------------    ------------    -----
 * VCC   ->  3.3V               3.3V            3.3V
 * GND   ->  GND                GND             GND
 * SDA   ->  A4                 20              21
 * SCL   ->  A5                 21              22
 * SDO   ->  GND (addr 0x68) or VCC (addr 0x69)
 * CS    ->  VCC (enables I2C mode)
 * 
 * AUTHOR: Arduino Implementation Team
 * VERSION: 1.0
 * DATE: 2024
 * LICENSE: MIT
 * 
 * =============================================================================
 */

#ifndef BMI323_H
#define BMI323_H

//#include <Arduino.h>
//#include <Wire.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
// HARDWARE DEFINITIONS
//=============================================================================

// I2C Address Options
#define BMI323_I2C_ADDR_PRIMARY   0x68  ///< Primary I2C address (SDO -> GND)
#define BMI323_I2C_ADDR_SECONDARY 0x69  ///< Secondary I2C address (SDO -> VCC)

// BMI323 Register Map
#define BMI323_REG_CHIP_ID        0x00  ///< Chip identification register

#define BMI323_REG_STATUS         0x02  ///< Sensor status register
#define BMI323_REG_ACC_DATA_X     0x03  ///< Accelerometer X-axis data register
#define BMI323_REG_ACC_DATA_Y     0x04  ///< Accelerometer Y-axis data register
#define BMI323_REG_ACC_DATA_Z     0x05  ///< Accelerometer Z-axis data register
#define BMI323_REG_GYR_DATA_X     0x06  ///< Gyroscope X-axis data register
#define BMI323_REG_GYR_DATA_Y     0x07  ///< Gyroscope Y-axis data register
#define BMI323_REG_GYR_DATA_Z     0x08  ///< Gyroscope Z-axis data register
#define BMI323_REG_TEMP_DATA      0x09


#define BMI323_REG_ACC_CONF       0x20  ///< Accelerometer configuration register
#define BMI323_REG_GYR_CONF       0x21  ///< Gyroscope configuration register


#define BMI323_REG_CMD            0x7E  ///< Command register

// BMI323 Constants
#define BMI323_CHIP_ID_VALUE      0x43    ///< Expected chip ID value
#define BMI323_CMD_SOFT_RESET     0xDEAF  ///< Soft reset command value

// Conversion Factors
#define BMI323_ACCEL_SCALE_4G     8.19f   ///< Accelerometer scale factor for ±4g range
#define BMI323_GYRO_SCALE_1000DPS 32.768f ///< Gyroscope scale factor for ±1000dps range

//=============================================================================
// DATA STRUCTURES
//=============================================================================

/**
 * @brief BMI323 sensor data structure
 * 
 * Contains all sensor readings and status information.
 * Updated by bmi323_read_data() function.
 */
typedef struct {
    // Raw sensor data (16-bit signed values)
    int16_t acc_x;          ///< Raw accelerometer X-axis data
    int16_t acc_y;          ///< Raw accelerometer Y-axis data  
    int16_t acc_z;          ///< Raw accelerometer Z-axis data
    int16_t gyr_x;          ///< Raw gyroscope X-axis data
    int16_t gyr_y;          ///< Raw gyroscope Y-axis data
    int16_t gyr_z;          ///< Raw gyroscope Z-axis data
    int16_t temperature;    ///< Raw temperature data
    
    // Status and identification
    uint32_t sensor_time;   ///< Sensor timestamp (reserved for future use)
    uint8_t chip_id;        ///< Chip ID (should be 0x43)
    uint8_t status;         ///< Sensor status register value
} bmi323_data_t;

//=============================================================================
// INITIALIZATION & CONTROL FUNCTIONS
//=============================================================================

/**
 * @brief Initialize the BMI323 sensor
 * 
 * Performs complete sensor initialization sequence:
 * 1. Tests I2C communication on both possible addresses
 * 2. Verifies chip ID matches expected value (0x43)
 * 3. Performs soft reset to ensure clean state
 * 4. Configures accelerometer (100Hz ODR, ±4g range)
 * 5. Configures gyroscope (100Hz ODR, ±1000°/s range)
 * 6. Allows sensors to stabilize
 * 
 * @return true if initialization successful, false on any error
 * 
 * @note Must call Wire.begin() before calling this function
 * 
 * @example
 * Wire.begin();
 * if (bmi323_init()) {
 *     Serial.println("Sensor ready!");
 * } else {
 *     Serial.println("Init failed!");
 * }
 */
bool bmi323_init(void);

/**
 * @brief Check I2C connection and verify chip ID
 * 
 * Attempts to communicate with BMI323 on both possible I2C addresses.
 * Sets internal address variable when successful connection is established.
 * Verifies that the connected device is actually a BMI323 by checking chip ID.
 * 
 * @return true if sensor found and chip ID matches, false otherwise
 * 
 * @note This function is called automatically by bmi323_init()
 */
bool bmi323_check_connection(void);

//=============================================================================
// DATA READING FUNCTIONS
//=============================================================================

/**
 * @brief Read all sensor data from BMI323
 * 
 * Performs a complete sensor data read cycle:
 * - Status register
 * - All 3 accelerometer axes (X, Y, Z)
 * - All 3 gyroscope axes (X, Y, Z)  
 * - Temperature sensor
 * 
 * Updates the internal sensor data structure with fresh readings.
 * All getter functions return data from this internal structure.
 * 
 * @return true if all reads successful, false on any I2C communication error
 * 
 * @note Must call this function before using any getter functions to ensure fresh data
 * 
 * @example
 * if (bmi323_read_data()) {
 *     float accel_x = bmi323_get_accel_x_g();
 *     float gyro_z = bmi323_get_gyro_z_dps();
 * }
 */
bool bmi323_read_data(void);


bool bmi323_read_data_burst(void);

/**
 * @brief Print formatted sensor data to Serial
 * 
 * Outputs human-readable sensor data in a single line format:
 * "Accel: X=1.23g Y=-0.45g Z=0.98g | Gyro: X=12.3°/s Y=-5.7°/s Z=89.1°/s"
 *
 * @return None
 * 
 * @note Requires bmi323_read_data() to be called first for current data
 * @note Uses 2 decimal places for acceleration, 1 for gyroscope
 */
void bmi323_print_data(void);

/**
 * @brief Non-blocking continuous sensor reading loop
 * 
 * Designed to be called from Arduino loop() function.
 * Automatically reads sensor data every 100ms and prints to Serial.
 * Uses internal timing - does not block other code execution.
 * 
 * @return None
 * 
 * @note This is the equivalent of the original STM32 test loop function
 * 
 * @example
 * void loop() {
 *     bmi323_test_loop(); // Continuous sensor monitoring
 *     // Other code can run here
 * }
 */
void bmi323_test_loop(void);

//=============================================================================
// CONVERTED DATA GETTERS (Physical Units)
//=============================================================================


//=============================================================================
// STATUS & INFORMATION GETTERS
//=============================================================================

/**
 * @brief Get chip identification value
 * @return 8-bit chip ID (should always be 0x43 for BMI323)
 */
uint8_t bmi323_get_chip_id(void);



//=============================================================================
// ERROR CODES & DIAGNOSTICS
//=============================================================================

typedef enum {
    BMI323_OK = 0,              ///< No error
    BMI323_ERROR_I2C,           ///< I2C communication error
    BMI323_ERROR_CHIP_ID,       ///< Wrong chip ID
    BMI323_ERROR_NOT_INIT,      ///< Sensor not initialized
    BMI323_ERROR_TIMEOUT,       ///< Operation timeout
    BMI323_ERROR_CONFIG         ///< Configuration error
} bmi323_error_t;




#ifdef __cplusplus
}
#endif

#endif // BMI323_H