/*
 * =============================================================================
 * BMI323 6-Axis IMU Library for Arduino - Implementation
 * Pure C Implementation with Comprehensive Error Handling
 * =============================================================================
 * 
 * FILE: bmi323.c
 * 
 * DESCRIPTION:
 * Complete implementation of BMI323 sensor library for Arduino.
 * Provides low-level I2C communication, sensor initialization,
 * data reading, and utility functions.
 * 
 * FEATURES:
 * - Robust I2C communication with error handling
 * - Automatic sensor configuration and calibration
 * - Raw and converted data access
 * - Non-blocking operation support
 * - Comprehensive debugging information
 * 
 * AUTHOR: Arduino Implementation Team
 * VERSION: 1.0
 * DATE: 2024
 * LICENSE: MIT
 * 
 * =============================================================================
 */

#include "bmi323.h"

//=============================================================================
// PRIVATE VARIABLES
//=============================================================================

bmi323_data_t sensor_data;        ///< Internal sensor data storage
static bmi323_config_t config;           ///< Internal configuration storage

//=============================================================================
// PRIVATE FUNCTION DECLARATIONS
//=============================================================================

static bool bmi323_write_register(uint8_t reg, uint16_t data);
static bool bmi323_read_register(uint8_t reg, uint16_t* data);
static bool bmi323_soft_reset(void);
static void bmi323_set_error(bmi323_error_t error);

//=============================================================================
// PRIVATE I2C COMMUNICATION FUNCTIONS
//=============================================================================

/**
 * @brief Write 16-bit data to BMI323 register
 * 
 * BMI323 uses a 3-byte write protocol:
 * [Register Address][Data LSB][Data MSB]
 * 
 * @param reg Register address to write to
 * @param data 16-bit data value to write
 * @return true if write successful, false on I2C error
 */
static bool bmi323_write_register(uint8_t reg, uint16_t data) {
    Wire.beginTransmission(config.i2c_address);
    Wire.write(reg);                               // Register address
    Wire.write((uint8_t)(data & 0xFF));           // LSB first
    Wire.write((uint8_t)((data >> 8) & 0xFF));    // MSB second
    
    uint8_t result = Wire.endTransmission();
    if (result != 0) {
        return false;
    }
    return true;
}

/**
 * @brief Read 16-bit data from BMI323 register
 * 
 * BMI323 uses a 4-byte read protocol:
 * Send: [Register Address]
 * Receive: [Dummy][Dummy][Data LSB][Data MSB]
 * 
 * The actual data is in bytes 2 and 3 of the response.
 * 
 * @param reg Register address to read from
 * @param data Pointer to store 16-bit read data
 * @return true if read successful, false on I2C error
 */
static bool bmi323_read_register(uint8_t reg, uint16_t* data) {
    // Send register address
    Wire.beginTransmission(config.i2c_address);
    Wire.write(reg);
    
    uint8_t result = Wire.endTransmission();
    if (result != 0) {
        return false;
    }
    
    // Request 4 bytes according to BMI323 protocol
    Wire.requestFrom(config.i2c_address, (uint8_t)4);
    
    // Wait for data with timeout
    uint32_t timeout = millis() + 100; // 100ms timeout
    while (Wire.available() < 4 && millis() < timeout) {
        delay(1);
    }
    
    if (Wire.available() >= 4) {
        uint8_t buffer[4];
        for (int i = 0; i < 4; i++) {
            buffer[i] = Wire.read();
        }
        
        // Extract data from bytes 2 and 3 (LSB, MSB)
        *data = (uint16_t)(buffer[2] | (buffer[3] << 8));
        return true;
    } else {
        return false;
    }
}

/**
 * @brief Perform soft reset of BMI323 sensor
 * 
 * Sends the soft reset command and waits for sensor to restart.
 * This ensures the sensor is in a known clean state.
 * 
 * @param None
 * @return true if reset successful, false on error
 */
static bool bmi323_soft_reset(void) {
    
    if (bmi323_write_register(BMI323_REG_CMD, BMI323_CMD_SOFT_RESET)) {
        delay(50); // Wait for reset to complete
        return true;
    } else {
        return false;
    }
}

//=============================================================================
// PUBLIC INITIALIZATION FUNCTIONS
//=============================================================================

bool bmi323_check_connection(void) {
    uint16_t chip_id;
    
    // Try primary I2C address (0x68)
    config.i2c_address = BMI323_I2C_ADDR_PRIMARY;
    if (bmi323_read_register(BMI323_REG_CHIP_ID, &chip_id)) {
        if ((chip_id & 0xFF) == BMI323_CHIP_ID_VALUE) {
            sensor_data.chip_id = chip_id & 0xFF;
            Serial.println(sensor_data.chip_id, HEX);
            return true;
        }
    }
    return false;
}

bool bmi323_init(void) {
    
    // Initialize configuration structure
    memset(&config, 0, sizeof(config));
    memset(&sensor_data, 0, sizeof(sensor_data));
    
    // Check I2C connection and identify sensor
    if (!bmi323_check_connection()) {
        return false;
    }
    
    // Perform soft reset to ensure clean state
    if (!bmi323_soft_reset()) {
        return false;
    }
    
    // Verify connection after reset
    if (!bmi323_check_connection()) {
        return false;
    }
    
    // Configure accelerometer: 100Hz ODR, ±4g range
    if (!bmi323_write_register(BMI323_REG_ACC_CONF, 0x4618)) {
        return false;
    }
    
    // Configure gyroscope: 100Hz ODR, ±1000°/s range
    if (!bmi323_write_register(BMI323_REG_GYR_CONF, 0x4638)) {
        return false;
    }
    
    // Mark as successfully initialized
    config.initialized = true;
    config.last_read_time = millis();
    
    return true;
}

//=============================================================================
// PUBLIC DATA READING FUNCTIONS
//=============================================================================


bool bmi323_read_data_burst(void){
    const uint8_t blen = 22;
    uint16_t temp_data;
     //uint8_t buff[blen];
     uint8_t buffer[4];
     uint8_t result;
     int i,j;
    if (!config.initialized) {
        return false;
    }
        
    Wire.beginTransmission(config.i2c_address);
    Wire.write(BMI323_REG_STATUS);
    
    result = Wire.endTransmission();
    if (result != 0) {
        return false;
    }
    
    // Request 4 bytes according to BMI323 protocol
    Wire.requestFrom(config.i2c_address, (uint8_t)blen);
    
    // Wait for data with timeout
    uint32_t timeout = millis() + 100; // 100ms timeout
    while (Wire.available() < blen && millis() < timeout) {
        delay(1);
    }
    
     if (Wire.available() >= blen) {
       
        for ( i = 0, j= 0; i < 4; i++, j++) {
            buffer[i] = Wire.read();
        }
        sensor_data.status = buffer[2];//|buffer[3]<<8;


        for ( i = 0; i < 2; i++,j++) {
            buffer[i] = Wire.read();
        }
        sensor_data.acc_x = ((uint16_t)(buffer[0] | buffer[1]<<8));

        for ( i = 0; i < 2; i++,j++) {
            buffer[i] = Wire.read();
        }
        sensor_data.acc_y = ((uint16_t)(buffer[0] | buffer[1]<<8));

        for ( i = 0; i < 2; i++,j++) {
            buffer[i] = Wire.read();
        }
        sensor_data.acc_z = ((uint16_t)(buffer[0] | buffer[1]<<8));
//10

        for ( i = 0; i < 2; i++,j++) {
            buffer[i] = Wire.read();
        }
        sensor_data.gyr_x = ((uint16_t)(buffer[0] | buffer[1]<<8));

        for ( i = 0; i < 2; i++,j++) {
            buffer[i] = Wire.read();
        }
        sensor_data.gyr_y = ((uint16_t)(buffer[0] | buffer[1]<<8)); 

        for ( i = 0; i < 2; i++,j++) {
            buffer[i] = Wire.read();
        }
        sensor_data.gyr_z = ((uint16_t)(buffer[0] | buffer[1]<<8));

        for ( i = 0; i < 2; i++,j++) {
            buffer[i] = Wire.read();
        }
        sensor_data.temperature = ((uint16_t)(buffer[0] | buffer[1]<<8));
        
        for ( i = 0; i < 4; i++,j++) {
            buffer[i] = Wire.read();
        }
        // Serial.print( "J is:");
        // Serial.print(j);
        // Serial.println();
        // Serial.print(buffer[0],HEX);
        // Serial.print(" ");
        // Serial.print(buffer[1],HEX);
        // Serial.print(" ");
        // Serial.print(buffer[2],HEX);
        // Serial.print(" ");
        // Serial.println(buffer[3],HEX);
        sensor_data.sensor_time = ((uint32_t)buffer[0]);
        sensor_data.sensor_time |= ((uint32_t) buffer[1]<<8);
        sensor_data.sensor_time |= ((uint32_t) buffer[2]<<16);
        sensor_data.sensor_time |= ((uint32_t) buffer[3]<<24);
      


        return true;
        }else{
        Serial.println("err i2c rd");
      
    }
    return false;
}

bool bmi323_read_data(void) {
    if (!config.initialized) {
        return false;
    }
    
    uint16_t temp_data;
    
    // Read status register
    if (!bmi323_read_register(BMI323_REG_STATUS, &temp_data)) {
        return false;
    }
    sensor_data.status = temp_data & 0xFF;
    
    // Read accelerometer data (X, Y, Z)
    if (!bmi323_read_register(BMI323_REG_ACC_DATA_X, &temp_data)) {
        return false;
    }
    sensor_data.acc_x = (int16_t)temp_data;
    
    if (!bmi323_read_register(BMI323_REG_ACC_DATA_Y, &temp_data)) {
        return false;
    }
    sensor_data.acc_y = (int16_t)temp_data;
    
    if (!bmi323_read_register(BMI323_REG_ACC_DATA_Z, &temp_data)) {
        return false;
    }
    sensor_data.acc_z = (int16_t)temp_data;
    
    // Read gyroscope data (X, Y, Z)
    if (!bmi323_read_register(BMI323_REG_GYR_DATA_X, &temp_data)) {
        return false;
    }
    sensor_data.gyr_x = (int16_t)temp_data;
    
    if (!bmi323_read_register(BMI323_REG_GYR_DATA_Y, &temp_data)) {
        return false;
    }
    sensor_data.gyr_y = (int16_t)temp_data;
    
    if (!bmi323_read_register(BMI323_REG_GYR_DATA_Z, &temp_data)) {
        return false;
    }
    sensor_data.gyr_z = (int16_t)temp_data;

    if(!bmi323_read_register(BMI323_REG_TEMP_DATA, &temp_data)){
        return false;
    }
    
    sensor_data.temperature = (uint16_t) temp_data;

    // Update last successful read time
    config.last_read_time = millis();
    
    return true;
}

void bmi323_print_data(void) {
    if (!config.initialized) {
        Serial.println("Error: Sensor not initialized!");
        return;
    }
    
    // Convert raw data to physical units
    float acc_x_g = sensor_data.acc_x / BMI323_ACCEL_SCALE_4G / 1000.0f;
    float acc_y_g = sensor_data.acc_y / BMI323_ACCEL_SCALE_4G / 1000.0f;
    float acc_z_g = sensor_data.acc_z / BMI323_ACCEL_SCALE_4G / 1000.0f;
    
    float gyr_x_dps = sensor_data.gyr_x / BMI323_GYRO_SCALE_1000DPS;
    float gyr_y_dps = sensor_data.gyr_y / BMI323_GYRO_SCALE_1000DPS;
    float gyr_z_dps = sensor_data.gyr_z / BMI323_GYRO_SCALE_1000DPS;
    float temperature = sensor_data.temperature*0.01f;
    
    Serial.print("Status:");
    Serial.println(sensor_data.status,HEX);
    // Print formatted data
    Serial.print("Accel: X=");
    Serial.print(acc_x_g, 2);
    Serial.print("g Y=");
    Serial.print(acc_y_g, 2);
    Serial.print("g Z=");
    Serial.print(acc_z_g, 2);
    Serial.print("g | Gyro: X=");
    Serial.print(gyr_x_dps, 1);
    Serial.print("°/s Y=");
    Serial.print(gyr_y_dps, 1);
    Serial.print("°/s Z=");
    Serial.print(gyr_z_dps, 1);
    Serial.print("°/s Tempreature:");
    Serial.print(sensor_data.temperature);
    Serial.print(" C Time:");
    Serial.println(sensor_data.sensor_time);
}

void bmi323_test_loop(void) {
    static uint32_t last_read = 0;
    
    // Read sensor data every 100ms (10Hz output rate)
    if (millis() - last_read >= 1000) {
        // if (bmi323_read_data()) {
        //     bmi323_print_data();
        // }
        bmi323_read_data_burst();
         bmi323_print_data();
        last_read = millis();
    }
}

//=============================================================================
// PUBLIC STATUS & INFORMATION GETTERS
//=============================================================================

uint8_t bmi323_get_chip_id(void) {
    return sensor_data.chip_id;
}