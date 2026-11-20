/**
 * @file nonvolatile.h
 * @author Martin Zůbek
 * @brief Header file for non-volatile storage configuration provided by the user.
 * @date 2025-12-17
 */
#ifndef NONVOLATILE_H
#define NONVOLATILE_H

#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"

/**
 * @struct USER_CONFIG_T
 * @brief Structure to store provided configuration data from user for non-volatile storage.
 */
typedef struct{
    // Minimum threshold for brightness
    float minThreshold;
    // Maximum threshold for brightness
    float maxThreshold;
    // Hysteresis count for debouncing
    unsigned int hysteresisCount;
    // Time for next measurement in milliseconds
    int timeForNextMeasurement;
} USER_CONFIG_T;


/**
 * @name nonvolatileInit
 * @brief Funtion for initializing non-volatile storage.
 * @return esp_err_t esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t nonvolatileInit(void);

/**
 * @name nonvolatileSaveDefaults
 * @brief Function for storing default configuration into non-volatile storage.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t nonvolatileSaveDefaults();

/**
 * @name getNonvolatileData
 * @brief Function for getting configuration data from non-volatile storage.
 * @param saved_configuration Pointer to the structure where will be stored configuration data.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t getNonvolatileData(USER_CONFIG_T *configuration_storage);

/**
 * @name nonvolatileSave
 * @brief Function for saving configuration data to non-volatile storage.
 * @param configuration Pointer to the configuration data to be saved.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t nonvolatileSave(USER_CONFIG_T *configuration);

#endif // NONVOLATILE_H
