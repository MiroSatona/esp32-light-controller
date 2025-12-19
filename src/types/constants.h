/**
 * @file constants.h
 * @author Martin Zůbek, 
 * @brief Header file for project constants and macros.
 * @date 2025-12-16
 */

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <stdio.h>
#include "driver/gpio.h"
#include "driver/ledc.h"

/**
 * =============================================================
 *                     LED CONFIGURATION
 * =============================================================
 */

// GPIO pin of LED on the PIN 25
#define LED_GPIO_PIN_25         GPIO_NUM_25 
// GPIO pin OF LED ON the PIN 26  
#define LED_GPIO_PIN_26         GPIO_NUM_26
// LED PWM frequency in Hz
#define LED_FREQUENCY           5000
// LED channel for LED on pin 25
#define LED_CHANNEL_25          LEDC_CHANNEL_0       
// LED channel for LED on pin 26
#define LED_CHANNEL_26          LEDC_CHANNEL_1
// Size of PWM resolution in bits
#define LED_PWM_BIT_SIZE        13
// Maximum PWM value
#define LED_PWM_MAX             ((1 << LED_PWM_BIT_SIZE) - 1)
// Minimum PWM value
#define LED_PWM_MIN             0
// Minimum hysteresis debounce samples
#define HYSTERESIS_COUNT_MIN    4
// Maximum hysteresis debounce samples
#define DEFAULT_HYSTERESIS_COUNT_MAX    10
// Small interval threshold
#define TOO_SMALL_INTERVAL      0.0001f

/**
 * ============================================================= 
 *                   LOGS CONFIGURATION
 * =============================================================
 */

// Log tag for main module
#define MAIN_LOG          "MAIN"
// Log tag for LED module
#define LED_LOG           "LED"
// Log tag for sensor module
#define SENSOR_LOG        "SENSOR"
// Log tag for WiFi module
#define WIFI_LOG          "WIFI"
// Log tag for non-volatile storage module
#define NONVOLATILE_LOG   "NONVOL"

/**
 * ============================================================= 
 *                   SCIENTIFIC CONSTANTS  
 * =============================================================
 */

// Constant for gamma correction
#define GAMMA_REDUCTION    2.2f
// Default maximum lux value for sensored light
#define DEFAULT_LUX_MAX    120.0f
// Default minimum lux value for sensored light
#define DEFAULT_LUX_MIN    10.0f
// Constant for 100% percentage
#define HUNDRED_PERCENTAGE 1.0f
// Constant for 0% percentage
#define ZERO_PERCENTAGE    0.0f 

/**
 * ============================================================= 
 *                   BH1750 SENSOR CONSTANTS  
 * =============================================================
*/

// I2C port 
#define I2C_PORT                    0
// I2C SDA pin
#define I2C_SDA                     GPIO_NUM_21
// I2C SCL pin
#define I2C_SCL                     GPIO_NUM_22
// BH1750 I2C address
#define I2C_ADDR                    0x23
// Glitch ignore count for I2C bus
#define I2C_IGNORE_GLITCH_CNT       7
// I2C bus frequency in Hz
#define I2C_FREQUENCY               100000
// Power on command 
#define CMD_POWER_ON                0x01
// Reset command
#define CMD_RESET                   0x07
// High resolution mode command
#define CMD_H_RES_MODE              0x10
// Time to wait after setting command in milliseconds
#define WAIT_TIME_AFTER_COMMAND     10 
// Time to wait for measurement in milliseconds
#define WAIT_TIME_FOR_MEASUREMENT   150
// Time for reading value form sensor in milliseconds
#define TIME_READING_SENSOR         1000
// Count of bytes to read from sensor
#define READ_LENGTH_BYTES           2
// Conversion factor from raw sensor value to lux from datasheet
#define BH1750_CONVERSION           1.2f

#endif // CONSTANTS_H