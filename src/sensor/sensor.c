/**
 * @file sensor.c
 * @author Martin Zůebk, 
 * @brief Source file for I2C bus and BH1750 sensor initialization and lux value reading from BH1750 sensor.
 * @date 2025-12-17
 */

#include "sensor.h"
#include "esp_err.h"
#include "esp_log.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "types/constants.h"

// Flag flag for initialization of I2C
static bool isI2cInit = false;
// Flag for initialization of BH1750 sensor
static bool isSensorInit = false;

// I2C bus
static i2c_master_bus_handle_t i2cBus = NULL;
// BH1750 device
static i2c_master_dev_handle_t sensorDevice = NULL;

esp_err_t setCommand(uint8_t commandToSet){
    return sensorDevice != NULL ? i2c_master_transmit(sensorDevice, &commandToSet, 1, pdMS_TO_TICKS(1000)) : ESP_ERR_INVALID_STATE;
}

esp_err_t sensorInit(void)
{   
    // Result of the initializations
    esp_err_t result = ESP_OK;

    // Initialization of device only once
    if (!isI2cInit) {


        // Structure for I2C bus configuration
        i2c_master_bus_config_t i2cConfiguration;
        memset(&i2cConfiguration, 0, sizeof(i2c_master_bus_config_t));
        
        // Configure I2C bus parameters
        i2cConfiguration.i2c_port = I2C_PORT;
        i2cConfiguration.sda_io_num = I2C_SDA;
        i2cConfiguration.scl_io_num = I2C_SCL;
        i2cConfiguration.clk_source = I2C_CLK_SRC_DEFAULT;
        i2cConfiguration.glitch_ignore_cnt = I2C_IGNORE_GLITCH_CNT;
        i2cConfiguration.flags.enable_internal_pullup = true;

        // Create new I2C master bus
        result = i2c_new_master_bus(&i2cConfiguration, &i2cBus);
        if (result != ESP_OK) {
            ESP_LOGE(SENSOR_LOG, "ERROR: Creation of new i2c master bus failed: %s", esp_err_to_name(result));
            return result;
        }

        // Structure for I2C device configuration
        i2c_device_config_t sensorConfiguration;
        memset(&sensorConfiguration, 0, sizeof(i2c_device_config_t));

        // Configure sensor device parameters
        sensorConfiguration.device_address = I2C_ADDR;
        sensorConfiguration.scl_speed_hz = I2C_FREQUENCY;

        // Add BH1750 device to the I2C bus
        result = i2c_master_bus_add_device(i2cBus, &sensorConfiguration, &sensorDevice);
        if (result != ESP_OK) {
            ESP_LOGE(SENSOR_LOG, "ERROR: Creation of new i2c master sensor device failed: %s", esp_err_to_name(result));
            return result;
        }

        // Set flag for initialized I2C
        isI2cInit = true;
        ESP_LOGI(SENSOR_LOG, "LOG: I2C bus was successfully initialized!");
    }

  

    // Set POWER ON command for BH1750
    result = setCommand(CMD_POWER_ON);
    if (result != ESP_OK) {
        ESP_LOGE(SENSOR_LOG, "ERROR: Set of POWER ON command failed: %s", esp_err_to_name(result));
        return result;
    }

    vTaskDelay(pdMS_TO_TICKS(WAIT_TIME_AFTER_COMMAND));

    // Set RESET command for BH1750 for clearing data register
    result = setCommand(CMD_RESET);
    if (result != ESP_OK) {
        ESP_LOGE(SENSOR_LOG, "ERROR: Set of RESET command failed: %s", esp_err_to_name(result));
        return result;
    }

    vTaskDelay(pdMS_TO_TICKS(WAIT_TIME_AFTER_COMMAND));

    // Set CONTINUOUS HIGH RESOLUTION MODE command for BH1750 for measurement
    result = setCommand(CMD_H_RES_MODE );
    if (result != ESP_OK) {
        ESP_LOGE(SENSOR_LOG, "ERROR: Set of CONT HI-RES MODE command failed: %s", esp_err_to_name(result));
        return result;
    }

    // Wait for measurement with one high resolution mode measurement with 30 ms extra time
    vTaskDelay(pdMS_TO_TICKS(WAIT_TIME_FOR_MEASUREMENT));

    // Set flag for initialized sensor
    isSensorInit = true;
    ESP_LOGI(SENSOR_LOG, "LOG: BH1750 on I2C bus was successfully initialized!");

    return ESP_OK;
}


esp_err_t readLuxValue(float *luxValueStorage){

    // Check existence of storage for lux value
    if (luxValueStorage == NULL) return ESP_ERR_INVALID_ARG;
    esp_err_t result = ESP_OK;

    // Initialize sensor if not initialized
    if (!isSensorInit) {
        result = sensorInit() == ESP_OK ? ESP_OK : ESP_ERR_INVALID_STATE;
        if (result != ESP_OK) return result;
    }

    // Prepare storage for received value
    uint8_t recivedValue[READ_LENGTH_BYTES];
    memset(recivedValue, 0, sizeof(READ_LENGTH_BYTES*sizeof(uint8_t)));

    // Read 2 bytes value form BH1750 sensor
    result = i2c_master_receive(sensorDevice, recivedValue, READ_LENGTH_BYTES, pdMS_TO_TICKS(TIME_READING_SENSOR));
    if (result != ESP_OK) {
        ESP_LOGE(SENSOR_LOG, "ERROR: Read of value on the sensor failed: %s", esp_err_to_name(result));
        return result;
    }

    // Combine two bytes to one 16-bit raw value
    uint16_t rawValueReaded = (uint16_t)((recivedValue[0] << 8) | recivedValue[1]);

    // Convert raw value to lux value using conversion factor from datasheet
    *luxValueStorage = ((float)rawValueReaded) / BH1750_CONVERSION;

    ESP_LOGI(SENSOR_LOG, "LOG: Readed lux value from BH1750 sensor: %.2f lux", *luxValueStorage);
    return ESP_OK;
}
