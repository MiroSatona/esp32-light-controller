/**
 * @file led.h
 * @author Martin Zůbek, 
 * @brief Header file for LEDs init and handle brightness and handle thresholds.
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
 * @name setBrightness
 * @brief Funtion for setting brightness of LEDs based on measured brightness and handle thresholds with hysteresis.
 * @param brightness The measured brightness value.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t setBrightness(float luxValue);

/**
 * @name setDuty
 * @brief Set on the both LEDS.
 * @param duty Duty which will be set.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type. 
 */
esp_err_t setDuty(uint32_t duty);

/**
 * @name setMinThreshold
 * @brief Function for setting minimum threshold for LEDs.
 * @param minThreshold The minimum threshold value to be set.
 */
void setMinThreshold(float minThreshold);

/**
 * @name setMaxThreshold
 * @brief Function for setting maximum threshold for LEDs.
 * @param maxThreshold The maximum threshold value to be set.
 */
void setMaxThreshold(float maxThreshold);

/**
 * @name getMaxThreshold
 * @brief Function for getting maximum threshold for LEDs.
 * @return float The current maximum threshold value.
 */
float getMinThreshold(void);

/**
 * @name getMinThreshold
 * @brief Function for getting minimum threshold for LEDs.
 * @return float The current minimum threshold value.
 */
float getMaxThreshold(void);

/**
 * @name getHysteresisStateAbove
 * @brief Function for getting current hysteresis state above for LEDs.
 * @return bool The current hysteresis state above.
 */
bool getHysteresisStateAbove(void);

/**
 * @name setHysteresisStateAbove
 * @brief Function for set hysteresis state above.
 * @param state Bool value indicate hysteresis state above.
 */
void setHysteresisStateAbove(bool state);

/**
 * @name getHysteresisStateBellow
 * @brief Function for getting current hysteresis state bellow for LEDs.
 * @return bool The current hysteresis state bellow.
 */
bool getHysteresisStateBellow(void);

/**
 * @name setHysteresisStateBellow
 * @brief Function for set hysteresis state bellow.
 * @param state Bool value indicate hysteresis state bellow.
 */
void setHysteresisStateBellow(bool state);

/**
 * @name getHysteresisSamples
 * @brief Function for getting current number of hysteresis samples for LEDs.
 * @return float The current number of hysteresis samples.
 */
unsigned int getHysteresisSamples(void);

/**
 * @name setHysteresisSamples
 * @brief Function for setting number of hysteresis samples for LEDs.
 * @param samples The number of hysteresis samples to be set.
 */
void setHysteresisSamples(unsigned int samples);

#endif // LED_H
