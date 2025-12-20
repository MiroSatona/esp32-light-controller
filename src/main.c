#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "types/constants.h"
#include "nonvolatile/nonvolatile.h"
#include "serial/serial.h"
#include "sensor/sensor.h"
#include "led/led.h"

void app_main(void){
       
    esp_log_level_set("*", ESP_LOG_WARN);
    esp_err_t result = ESP_OK;

    // Initialize non-volatile storage (if it fails, continue with defaults)
    result = nonvolatileInit();
    if (result != ESP_OK) {
        ESP_LOGW("MAIN", "WAR: Non-volatile storage init failed: %s. Using defaults.", esp_err_to_name(result));
    }

    // Initialize serial communication (CLI is optional)
    result = serialInit();
    if (result == ESP_OK) {
        // Run CLI in separate task (serialTask() is expected to loop forever)
        xTaskCreate((TaskFunction_t)serialTask, "serial_task", 4096, NULL, 5, NULL);
    } else {
        ESP_LOGW("MAIN", "WAR: Serial init failed: %s. Running without CLI.", esp_err_to_name(result));
    }

    // Initialize sensor (mandatory)
    result = sensorInit();
    if (result != ESP_OK) {
        ESP_LOGE("MAIN", "ERROR: Sensor initialization failed: %s.", esp_err_to_name(result));
        return;
    }

    // Initialize LEDs (mandatory)
    result = ledsInit();
    if (result != ESP_OK) {
        ESP_LOGE("MAIN", "ERROR: LED initialization failed: %s.", esp_err_to_name(result));
        return;
    }

    // Current user configuration
    USER_CONFIG_T currentConfiguration;
    memset(&currentConfiguration, 0, sizeof(USER_CONFIG_T));

    // Load current configuration from non-volatile storage
    result = getNonvolatileData(&currentConfiguration);
    if (result != ESP_OK) {
        ESP_LOGW("MAIN", "WAR: Loading configuration from NVS failed: %s. Using defaults.", esp_err_to_name(result));

        // Defaults (adjust names to your constants if needed)
        currentConfiguration.minThreshold = DEFAULT_LUX_MIN;
        currentConfiguration.maxThreshold = DEFAULT_LUX_MAX;
        currentConfiguration.hysteresisCount = HYSTERESIS_COUNT_MIN;
        currentConfiguration.timeForNextMeasurement = DEFAULT_TIME_FOR_NEXT_MEASUREMENT;
    }

    // Apply current configuration
    setMinThreshold(currentConfiguration.minThreshold);
    setMaxThreshold(currentConfiguration.maxThreshold);
    setHysteresisSamples(currentConfiguration.hysteresisCount);
    setTimeForNextMeasurement(currentConfiguration.timeForNextMeasurement);

    while (true) {
        float lux = 0.0f;

        // Read sensor + update LED
        if (readLuxValue(&lux) == ESP_OK) {
            setBrightness(lux);
        } else {
            ESP_LOGW("MAIN", "WAR: readLuxValue failed.");
        }

        // Delay according to config (fallback if broken)
        int periodMs = getTimeForNextMeasurement();
        if (periodMs <= 0) periodMs = DEFAULT_TIME_FOR_NEXT_MEASUREMENT;

        vTaskDelay(pdMS_TO_TICKS(periodMs));
    }
}
