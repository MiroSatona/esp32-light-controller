/**
 * @file wifi.h
 * @author Martin Zůbek
 * @brief Header file for WiFi functionality.
 * @date 2025-12-17
 */

#ifndef WIFI_H
#define WIFI_H

#include <stdio.h>
#include "esp_err.h"
#include "esp_event.h"

/**
 * @name wifiCallback
 * @brief Function to handle WiFi events.
 * @param context Pointer to user-defined context (not used).
 * @param eventBase Event base identifying the event source
 * @param eventId Identifier of the specific event.
 * @param eventData Pointer to additional event-specific data (not used).
 * @return void 
 */
void wifiCallback(void *context, esp_event_base_t eventBase, int32_t eventId, void *eventData);

/**
 * @name wifiInit
 * @brief Function to initialize WiFi connection.
 * @return esp_err_t - ESP_OK on success, otherwise some error of esp_err_t.
 */
esp_err_t wifiInit(void);

#endif // WIFI_H