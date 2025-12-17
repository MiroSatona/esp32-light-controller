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

// GPIO pin 25 for LED without threshold
#define LED_GPIO                GPIO_NUM_25 
// GPIO pin 26 for LED with threshold  
#define LED_THRESH_GPIO         GPIO_NUM_26
// LED PWM frequency in Hz
#define LED_FREQUENCY           5000
// LED channel for LED without threshold
#define LED_CHANNEL             LEDC_CHANNEL_0       
// LED channel for LED with threshold
#define LED_THRESH_CHANNEL      LEDC_CHANNEL_1
// Size of PWM resolution in bits
#define LED_PWM_BIT_SIZE        13
// Maximum PWM value
#define LED_PWM_MAX             ((1 << LED_PWM_BIT_SIZE) - 1)
// Minimum PWM value
#define LED_PWM_MIN             0

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
// Maximum lux value for sensored light
#define LUX_MAX            1000.0f
// Minimum lux value for sensored light
#define LUX_MIN            0.0f
// Constant for 100% percentage
#define HUNDRED_PERCENTAGE 1.0f
// Constant for 0% percentage
#define ZERO_PERCENTAGE    0.0f 


#endif // CONSTANTS_H