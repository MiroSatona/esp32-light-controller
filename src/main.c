/**
 * @file main.c
 * @author Martin Zůbek
 * @brief Main application file for the initilization and main loop.
 * @date 2025-12-17
 */

#include <stdio.h>
#include <stdbool.h>
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
#include "mqtt_wifi/wifi.h"
#include "mqtt_wifi/mqtt.h"

// Flag to indicate if is the system connected to WIFI
static bool IS_WIFI_CONNECTED = false;
// Flag to indicate if is the system connected to MQTT broker
static bool IS_MQTT_CONNECTED = false;
// Flag to indicate if serial communication is initialized
static bool IS_SERIAL_INITIALIZED = false;


/**
 * @name app_main
 * @brief Function initializes all modules and starts the main loop.
 * @return void 
 */
void app_main(void){
    
    // Set log level to WARNING and ERROR
    esp_log_level_set("*", ESP_LOG_WARN);

    esp_log_level_set(MAIN_LOG,        ESP_LOG_WARN);
    esp_log_level_set(LED_LOG,         ESP_LOG_WARN);
    esp_log_level_set(SENSOR_LOG,      ESP_LOG_WARN);
    esp_log_level_set(SERIAL_LOG,      ESP_LOG_WARN);
    esp_log_level_set(WIFI_LOG,        ESP_LOG_WARN);
    esp_log_level_set(MQTT_LOG,        ESP_LOG_WARN);
    esp_log_level_set(NONVOLATILE_LOG, ESP_LOG_WARN);

    // Variable to store functions results
    esp_err_t result = ESP_OK;

    // Initialize non-volatile storage
    result = nonvolatileInit();
    if (result != ESP_OK) {
        ESP_LOGE(MAIN_LOG, "ERROR: Non-volatile storage initialization failed: %s. Program will end.", esp_err_to_name(result));
        return;
    }

    // Initialize sensor
    result = sensorInit();
    if (result != ESP_OK) {
        ESP_LOGE(MAIN_LOG, "ERROR: Sensor initialization failed: %s. Program will end.", esp_err_to_name(result));
        return;
    }

    // Initialize LEDs
    result = ledsInit();
    if (result != ESP_OK) {
        ESP_LOGE(MAIN_LOG, "ERROR: LED initialization failed: %s. Program will end.", esp_err_to_name(result));
        return;
    }

    // Initialize WIFI
    result = wifiInit();
    if (result != ESP_OK) {
        ESP_LOGW(MAIN_LOG, "WAR: WIFI initialization failed: %s. Program will not send data to MQTT broker.", esp_err_to_name(result));
        IS_WIFI_CONNECTED = false;
    } else {
        IS_WIFI_CONNECTED = true;
    }

    // Delay to ensure WIFI is connected
    vTaskDelay(pdMS_TO_TICKS(WIFI_MQTT_CONNECTED_DELAY_MS));

    // Initialize MQTT
    if (IS_WIFI_CONNECTED) {
        result = mqttInit();
        if (result != ESP_OK) {
            ESP_LOGW(MAIN_LOG, "WAR: MQTT initialization failed: %s. Program will not send data to MQTT broker.", esp_err_to_name(result));
            IS_MQTT_CONNECTED = false;
        } else {
            IS_MQTT_CONNECTED = true;
        }
    }

    // Delay to ensure MQTT is connected
    vTaskDelay(pdMS_TO_TICKS(WIFI_MQTT_CONNECTED_DELAY_MS));

    // Initialize serial communication
    result = serialInit();
    if (result != ESP_OK) {
        ESP_LOGW(MAIN_LOG, "WAR: Serial communication initialization failed: %s. Program will run without serial communication.", esp_err_to_name(result));
        IS_SERIAL_INITIALIZED = false;
    } else {
        IS_SERIAL_INITIALIZED = true;
        // Start serial task
        xTaskCreate((TaskFunction_t)serialTask, "serial_task", 4096, NULL, 5, NULL);
    }

    // Reset result variable
    result = ESP_OK;

    // Main loop
    while (true) {

        // Variable to store measured lux value
        float measuredLux = 0.0f;

        // Read lux value from sensor
        result = readLuxValue(&measuredLux);
        if (result != ESP_OK) {
            ESP_LOGW(MAIN_LOG, "WAR: Reading lux value failed: %s.", esp_err_to_name(result));
        } else {
            // Set LED brightness based on measured lux
            setBrightness(measuredLux);
        }

        if(IS_WIFI_CONNECTED && IS_MQTT_CONNECTED && result == ESP_OK && mqttIsConnected()) {
            // Send measured lux value via MQTT
            result = mqttSend(measuredLux);
            if (result != ESP_OK) {
                ESP_LOGW(MAIN_LOG, "WAR: Sending lux value via MQTT failed: %s.", esp_err_to_name(result));
            }
        }

        // Delay for the configured time before next measurement
        int periodForNextMeasurement = getTimeForNextMeasurement();
        if (periodForNextMeasurement <= 0) periodForNextMeasurement = DEFAULT_TIME_FOR_NEXT_MEASUREMENT;
        vTaskDelay(pdMS_TO_TICKS(periodForNextMeasurement));

    }
}
