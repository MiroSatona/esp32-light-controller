#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "esp_err.h"


typedef enum {
    VALUE_TMIN,
    VALUE_TMAX,
    VALUE_DEBOUNCE,
    VALUE_TIME
} VALUE_TYPE_T;

/**
 * @name serialInit
 * @brief Function to initialize serial communication.
 * @return esp_err_t - ESP_OK on success, otherwise some 
 */
esp_err_t serialInit(void);
void printShow(void);
void printHelp(void);
bool isValidValue(VALUE_TYPE_T type, float floatValue, int integerValue);
void serialTask();


#endif // SERIAL_H