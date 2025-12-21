/**
 * @file serial.h
 * @author Martin Zůbek, 
 * @brief Header file for serial communication by UART.
 * @date 2025-12-17
 */
#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "esp_err.h"


/**
 * @enum VALUE_TYPE_T
 * @brief Enumeration for different types of configuration values.
 */
typedef enum {
    // Minimum threshold
    VALUE_TMIN,
    // Maximum threshold
    VALUE_TMAX,
    // Hysteresis debounce
    VALUE_DEBOUNCE,
    // Measurement interval
    VALUE_TIME
} VALUE_TYPE_T;

/**
 * @name serialInit
 * @brief Function to initialize serial communication.
 * @return esp_err_t - ESP_OK on success, otherwise some error of esp_err_t.
 */
esp_err_t serialInit(void);

/**
 * @name printShow
 * @brief Function to print the current user configuration.
 * @return void
 */
void printShow(void);

/**
 * @name printHelp
 * @brief Function to print help message.
 * @return void
 */
void printHelp(void);

/**
 * @name serialPrint
 * @brief Function to print text to serial.
 * @param textToPrint - String to print.
 * @return void
 */
void serialPrint(char *textToPrint);

/**
 * @name isValidValue
 * @brief Function to validate a provided value based on type of command.
 * @param type VALUE_TYPE_T - Type of value to validate.
 * @param floatValue float value provided.
 * @param integerValue integer value provided.
 * @return bool - true if the value is valid, false otherwise.
 */
bool isValidValue(VALUE_TYPE_T type, float floatValue, int integerValue);

/**
 * @name serialTask
 * @brief Task function for handling serial communication independent on user commands.
 * @return void
 */
void serialTask(void);

#endif // SERIAL_H