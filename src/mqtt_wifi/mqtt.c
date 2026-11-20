/**
 * @file mqtt.c
 * @author Martin Zůbek
 * @brief Source file for MQTT functionality.
 * @date 2025-12-17
 */

#include "mqtt.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "../types/constants.h"
#include "nonvolatile/nonvolatile.h"

// Strucutre for MQTT client
static esp_mqtt_client_handle_t client = NULL;
// Flag indicating MQTT connection status
static volatile bool isConnected = false;

void mqttCallback(void *context, esp_event_base_t eventBase, int32_t eventId, void *eventData){
    
    // Turn off unused parameter warnings -> required signature function
    (void)context;
    (void)eventBase;
    (void)eventData;

    // Handle different MQTT events
    switch ((esp_mqtt_event_id_t)eventId) {

        // MQTT connected event
        case MQTT_EVENT_CONNECTED:{ 
            isConnected = true;
            ESP_LOGI(MQTT_LOG, "LOG: Mqqt broker is connected.");
            break;
        }
        // MQTT disconnected event
        case MQTT_EVENT_DISCONNECTED: { 
            isConnected = false;
            ESP_LOGI(MQTT_LOG, "LOG: Mqqt broker is disconnected.");
            break;
        }
        // MQTT error event
        case MQTT_EVENT_ERROR: {
            isConnected = false;
            ESP_LOGE(MQTT_LOG, "ERROR: Error of mqtt connection.");
            break;
        }

        // Other MQTT events
        default: { 
            break;
        }
    }
}

esp_err_t mqttInit(void){
    
    // Result of the function
    esp_err_t result = ESP_OK;

    // MQTT client configuration
    esp_mqtt_client_config_t clientConfiguration;
    memset(&clientConfiguration, 0, sizeof(esp_mqtt_client_config_t));
    clientConfiguration.broker.address.uri = MQTT_URI;

    // Initialize MQTT client
    client = esp_mqtt_client_init(&clientConfiguration);
    if (!client) {
        ESP_LOGE(MQTT_LOG, "ERROR: Initialization of MQTT client failed.");
        return ESP_FAIL;
    }

    // Register MQTT event handler
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqttCallback, NULL);

    // Start MQTT client
    result = esp_mqtt_client_start(client);
    if (result != ESP_OK) {
        ESP_LOGE(MQTT_LOG, "ERROR: Starting of MQTT client failed: %s", esp_err_to_name(result));
        return result;
    }

    ESP_LOGI(MQTT_LOG, "LOG: MQTT client initialized and started successfully.");
    return ESP_OK;
}



esp_err_t mqttSend(float measuredLux){

    // Check if client is initialized and connected
    if (!client || !isConnected) {
        ESP_LOGW(MQTT_LOG, "WAR: MQTT client is not connected, cannot send message!");
        return ESP_ERR_INVALID_STATE;
    }

    // Load user configuration
    USER_CONFIG_T userConfiguration;
    memset(&userConfiguration, 0, sizeof(USER_CONFIG_T));
    if(getNonvolatileData(&userConfiguration) != ESP_OK){
        ESP_LOGE(MQTT_LOG, "ERROR: Loading user configuration failed, cannot send message!");
        return ESP_FAIL;
    }

    // Prepare JSON payload
    char jsonPayload[JSON_PAYLOAD_SIZE];
    memset(jsonPayload, 0, sizeof(jsonPayload));

    // Create JSON payload with measured lux and user configuration and get its length
    int payloadLength = snprintf(
        jsonPayload,
        sizeof(jsonPayload),
        "{\"lux_mes\":%.2f,\"interval_ms\":%d,\"min_lux\":%.2f,\"max_lux\":%.2f,\"hyst_samples\":%d}",
        (float)measuredLux,
        (int)userConfiguration.timeForNextMeasurement,
        (float)userConfiguration.minThreshold,
        (float)userConfiguration.maxThreshold,
        (int)userConfiguration.hysteresisCount
    );

    if (payloadLength < 0 || payloadLength >= (int)sizeof(jsonPayload)) {
        ESP_LOGE(MQTT_LOG, "ERROR: Creating JSON payload failed!");
        return ESP_FAIL;
    }

    // Publish JSON payload to MQTT topic
    int messageId = esp_mqtt_client_publish(client, MQTT_TOPIC_LUX, jsonPayload, 0, 1, 0);
    if (messageId < 0) {
        ESP_LOGE(MQTT_LOG, "ERROR: Publishing MQTT message failed!");
        return ESP_FAIL;
    }

    ESP_LOGI(MQTT_LOG, "LOG: MQTT published payload=%s (msg_id=%d) successfully.", jsonPayload, messageId);
    return ESP_OK;
}

bool mqttIsConnected(void){
    return isConnected;
}
