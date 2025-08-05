/*
 * =============================================================================
 * BMI323 6-Axis IMU Arduino Example - Complete Implementation
 * Pure C Implementation with Advanced Features
 * =============================================================================
 * 
 * FILE: AccelGyro.ino
 * 
 * DESCRIPTION:
 * Complete example demonstrating all features of the BMI323 library.
 * Includes basic data reading, motion detection, orientation sensing,
 * and advanced sensor analysis functions.
 * 
 * REQUIRED FILES:
 * AccelGyro/
 * ├── AccelGyro.ino (this file)
 * ├── bmi323.h      (header file)
 * └── bmi323.c      (implementation file)
 * 
 * HARDWARE CONNECTIONS:
 * BMI323 Pin    Arduino Uno/Nano    Arduino Mega    ESP32/ESP8266
 * ----------    ----------------    ------------    -------------
 * VCC       ->  3.3V               3.3V            3.3V
 * GND       ->  GND                GND             GND  
 * SDA       ->  A4                 20              21/4
 * SCL       ->  A5                 21              22/5
 * SDO       ->  GND (addr 0x68) or VCC (addr 0x69)
 * CS        ->  VCC (enables I2C mode)
 * 
 * FEATURES DEMONSTRATED:
 * - Basic sensor initialization and data reading
 * - Continuous monitoring with non-blocking loop
 * - Motion and rotation detection algorithms
 * - Device orientation detection
 * - Temperature monitoring
 * - Error handling and diagnostics
 * - Raw data access for custom processing
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
// CONFIGURATION CONSTANTS
//=============================================================================

// Timing configuration
#define MOTION_CHECK_INTERVAL    50     ///< Motion detection check interval (ms)

// Motion detection thresholds
#define MOTION_ACCEL_THRESHOLD   0.3f   ///< Acceleration threshold for motion (g)
#define ROTATION_GYRO_THRESHOLD  30.0f  ///< Gyroscope threshold for rotation (°/s)
#define FREEFALL_THRESHOLD       0.1f   ///< Threshold for freefall detection (g)
#define SHOCK_THRESHOLD          3.0f   ///< Threshold for shock detection (g)

//=============================================================================
// GLOBAL VARIABLES
//=============================================================================

// Timing variables
static unsigned long last_sensor_read = 0;
static unsigned long last_motion_check = 0;
static unsigned long last_temp_read = 0;
static unsigned long last_status_print = 0;

// Motion detection variables
static bool motion_detected = false;
static bool rotation_detected = false;
static char current_orientation = '?';

//=============================================================================
// ARDUINO SETUP FUNCTION
//=============================================================================

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);
    while (!Serial) {
        delay(10); // Wait for serial port to connect (needed for some boards)
    }
    
    Wire.begin();
    Wire.setClock(400000);
    
    // Initialize BMI323 sensor
    Serial.println("\nInitializing BMI323 sensor...");
    if (bmi323_init()) {
        Serial.println("✓ BMI323 sensor initialized successfully!");
        
        // Print sensor information
        Serial.print("Chip ID: 0x");
        Serial.println(bmi323_get_chip_id(), HEX);
        Serial.println("I2C Address:0x69");
        
    } else {
        Serial.println("Failed to initialize BMI323 sensor!");        
        // Halt execution with error indication
        while (1) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
        }
    }
    
    // Initialize LED for status indication
    pinMode(LED_BUILTIN, OUTPUT);
    
    Serial.println("Starting continuous sensor monitoring...");
    Serial.println();
}

//=============================================================================
// ARDUINO MAIN LOOP
//=============================================================================

void loop() {
  bmi323_test_loop();
}