/**
 * @file sensor.h
 * @author Martin Zůbek
 * @brief Header file for I2C bus and BH1750 sensor initialization and lux value reading from BH1750 sensor.
 * @date 2025-12-17
 */

#ifndef SENSOR_H
#define SENSOR_H

#include <stdio.h>
#include "esp_err.h"

/**
 * @name sensorInit
 * @brief Funtion for initialization of I2C bus and BH1750 sensor.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t sensorInit(void);

/**
 * @name readLuxValue
 * @brief Function for reading lux value from BH1750 sensor.
 * @param luxValueStorage Pointer to storage for readed lux value.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t readLuxValue(float *luxValueStorage);

/**
 * 
 * @name setCommand
 * @brief Function for setting command to BH1750 sensor.
 * @param commandToSet Command byte to send to the sensor.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t type.
 */
esp_err_t setCommand(uint8_t commandToSet);

#endif // SENSOR_H
