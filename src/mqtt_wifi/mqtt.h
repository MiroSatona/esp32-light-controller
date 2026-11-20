/**
 * @file mqtt.h
 * @author Martin Zůbek
 * @brief Header file for MQTT functionality.
 * @date 2025-12-17
 */

#ifndef MQTT_H
#define MQTT_H

#include <stdio.h>
#include "esp_err.h"
#include "esp_event.h"

/**
 * @name mqttCallback
 * @brief Function for handling MQTT events.
 * @param context Pointer to user-defined context (not used).
 * @param eventBase Event base identifying the event source.
 * @param eventId Identifier of the specific Wi-Fi or IP event.
 * @param eventData Pointer to additional event-specific data (not used).
 * @return void
 */
void mqttCallback(void *context, esp_event_base_t eventBase, int32_t eventId, void *eventData);

/**
 * @name mqttInit
 * @brief Function to initialize MQTT connection.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t.
 */
esp_err_t mqttInit(void);

/**
 * @name mqttIsConnected
 * @brief Function to check if MQTT is connected.
 * @return bool - true if connected, false otherwise.
 */
bool mqttIsConnected(void);

/**
 * @name mqttSend
 * @brief Function to send lux value via MQTT.
 * @param lux Float value representing the measured lux to be sent.
 * @return esp_err_t ESP_OK on success, otherwise some error of esp_err_t.
 */
esp_err_t mqttSend(float lux);

#endif // MQTT_H