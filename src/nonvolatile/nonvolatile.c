/**
 * @file nonvolatile.c
 * @author Martin Zůbek, 
 * @brief Source file for non-volatile storage configuration provided by the user.
 * @date 2025-12-17
 */

#include "nvs.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "nonvolatile.h"
#include "types/constants.h"


esp_err_t nonvolatileInit(void){

    // Initialize NVS flash
    esp_err_t result  = nvs_flash_init();

    // Everything ok
    if(result == ESP_OK){
        ESP_LOGI(NONVOLATILE_LOG, "LOG: Nonovolatile storage was initialized successfully!");
    }
    // Error due to no free pages or new version found -> NVS will be not used
    else if(result == ESP_ERR_NVS_NO_FREE_PAGES || result == ESP_ERR_NVS_NEW_VERSION_FOUND){
        ESP_LOGW(NONVOLATILE_LOG, "WAR: Nonvalolatile storage init failed due to no free pages or new version found.");
    }
    // Other error
    else{
        ESP_LOGE(NONVOLATILE_LOG, "ERROR: Nonvolatile storage init failed with error: %s", esp_err_to_name(result));
    }

    return result;
}


esp_err_t nonvolatileSave(USER_CONFIG_T *configuration){

    // Validate existence of inputed configuration
    if (configuration == NULL) return ESP_ERR_INVALID_ARG;

    // Result of the function
    esp_err_t result = ESP_OK;

    // Handler for NVS storage
    nvs_handle_t storageHandler;
    memset(&storageHandler, 0, sizeof(nvs_handle_t));

    // Open NVS storage
    result = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &storageHandler);
    if (result != ESP_OK) {
        ESP_LOGE(NONVOLATILE_LOG, "ERROR: Open of the nonvolatile storage failed: %s", esp_err_to_name(result));
        return result;
    }

    // Set blob data
    result = nvs_set_blob(storageHandler, NVS_KEY, configuration, sizeof(USER_CONFIG_T));
    if (result != ESP_OK) {
        ESP_LOGE(NONVOLATILE_LOG, "ERROR: Setting blob failed: %s", esp_err_to_name(result));
        nvs_close(storageHandler);
        return result;
    }

    // Commit changes and close storage
    result = nvs_commit(storageHandler);
    nvs_close(storageHandler);
    if (result != ESP_OK) {
        ESP_LOGE(NONVOLATILE_LOG, "ERROR: Committing changes failed: %s", esp_err_to_name(result));
        return result;
    }


    ESP_LOGI(NONVOLATILE_LOG,
        "Configuration saved to nonvolatile storage:\n"
        "  min threshold            = %.2f\n"
        "  max threshold            = %.2f\n"
        "  hysteresis debounce      = %u\n"
        "  period for next measurement  = %d ms\n"
        "----------------------------------------",
        configuration->minThreshold,
        configuration->maxThreshold,
        configuration->hysteresisCount,
        configuration->timeForNextMeasurement
    );

    return ESP_OK;
}


esp_err_t nonvolatileSaveDefaults(){

    // Prepare stucture for default values
    USER_CONFIG_T defaultConfiguration;
    memset(&defaultConfiguration, 0, sizeof(USER_CONFIG_T));

    // Set default values
    defaultConfiguration.minThreshold = DEFAULT_LUX_MIN;
    defaultConfiguration.maxThreshold = DEFAULT_LUX_MAX;
    defaultConfiguration.hysteresisCount = HYSTERESIS_COUNT_MIN;            
    defaultConfiguration.timeForNextMeasurement = DEFAULT_TIME_FOR_NEXT_MEASUREMENT;

    // Save to nonvolatile storage
    esp_err_t result  = nonvolatileSave(&defaultConfiguration);

    // Handle result
    if (result != ESP_OK) {
        ESP_LOGE(NONVOLATILE_LOG, "ERROR: Saving of defaults values to nonvolatile storage failed: %s", esp_err_to_name(result));
    } else {
        ESP_LOGI(NONVOLATILE_LOG, "LOG: Default values saved to nonvolatile storage successfully!");
    }

    return result;
}



esp_err_t getNonvolatileData(USER_CONFIG_T *configuration_storage){

    // Validate existence of configuration storage
    if (configuration_storage == NULL) return ESP_ERR_INVALID_ARG;

    // Result of the function
    esp_err_t result = ESP_OK;

    // Handler for NVS storage
    nvs_handle_t storageHandler;
    memset(&storageHandler, 0, sizeof(nvs_handle_t));

    // Open NVS storage
    result = nvs_open(NVS_NAMESPACE, NVS_READONLY, &storageHandler);

    // Namespace does not exist
    if (result == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(NONVOLATILE_LOG, "WAR: Nonovolatile storage was used for the first time, storing defaults configuration...");
        return nonvolatileSaveDefaults();
    }

    // Other error while reading
    if (result != ESP_OK) {
        ESP_LOGE(NONVOLATILE_LOG, "Readed data of the nonvolatile storage failed: %s", esp_err_to_name(result));
        return result;
    }

    // Get size of confiuguration
    size_t sizeOfSavedConfiguration = sizeof(USER_CONFIG_T);
    // Read blob data
    result = nvs_get_blob(storageHandler, NVS_KEY, configuration_storage, &sizeOfSavedConfiguration);
    nvs_close(storageHandler);

    // Not found configuration in NVS
    if (result == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW(NONVOLATILE_LOG, "WAR: Nonovolatile blob was used for the first time, storing defaults configuration...");
        return nonvolatileSaveDefaults();
    }

    // Other error while reading configuration from NVS
    if (result != ESP_OK) {
        ESP_LOGE(NONVOLATILE_LOG, "ERROR: Cannot read the configuration from NVS: %s", esp_err_to_name(result));
        return result;
    }

    // Validate size of readed configuration
    if (sizeOfSavedConfiguration != sizeof(USER_CONFIG_T)) {
        ESP_LOGW(NONVOLATILE_LOG, "WAR: Invalid blob size (%u), resetting stored configuration.", (unsigned)sizeOfSavedConfiguration);
        return nonvolatileSaveDefaults();
    }

    ESP_LOGI(NONVOLATILE_LOG,
        "Configuration readed from nonvolatile storage:\n"
        "  min threshold            = %.2f\n"
        "  max threshold            = %.2f\n"
        "  hysteresis debounce      = %u\n"
        "  period for next measurement  = %d ms\n"
        "----------------------------------------",
        configuration_storage->minThreshold,
        configuration_storage->maxThreshold,
        configuration_storage->hysteresisCount,
        configuration_storage->timeForNextMeasurement
    );

    return result;
}