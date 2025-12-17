/**
 * @file led.h
 * @author Martin Zůbek, 
 * @brief Header file for LED init and handle brightness by threshold or without threshold.
 * @date 2025-12-16
 */
#ifndef LED_H
#define LED_H

#include <stdio.h>
#include "esp_err.h"
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/ledc.h"

/**
 * @name ledsInit
 * @brief Funtion for initialiting channels for controlling LEDs by PWM and their timer.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t ledsInit(void);

/**
 * @name setBrightnessWithNoThreshold
 * @brief Funtion for setting brightness of LED without threshold based on measured brightness.
 * @param brightness The measured brightness value.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t setBrightnessWithNoThreshold(uint32_t brightness);

/**
 * @name setBrightnessWithThreshold
 * @brief Funtion for setting brightness of LED with threshold based on measured brightness.
 * @param brightness The measured brightness value.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t setBrightnessWithThreshold(uint32_t brightness);

/**
 * @name setMinThreshold
 * @brief Function for setting minimum threshold for LED with threshold.
 * @param minThreshold The minimum threshold value to be set.
 */
void setMinThreshold(float minThreshold);

/**
 * @name setMaxThreshold
 * @brief Function for setting maximum threshold for LED with threshold.
 * @param maxThreshold The maximum threshold value to be set.
 */
void setMaxThreshold(float maxThreshold);

/**
 * @name getMaxThreshold
 * @brief Function for getting maximum threshold for LED with threshold.
 * @return float The current maximum threshold value.
 */
float getMinThreshold(void);

/**
 * @name getMinThreshold
 * @brief Function for getting minimum threshold for LED with threshold.
 * @return float The current minimum threshold value.
 */
float getMaxThreshold(void);

/**
 * @name getHysteresisState
 * @brief Function for getting current hysteresis state for LED with threshold.
 * @return bool The current hysteresis state.
 */
bool getHysteresisState(void);

#endif // LED_H
