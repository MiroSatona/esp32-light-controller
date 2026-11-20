/**
 * @file wifi.c
 * @author Martin Zůbek
 * @brief Implementation file for WiFi functionality.
 * @date 2025-12-17
 */

#include "wifi.h"
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "../types/constants.h"

// Strucure with network interface of WiFi
static esp_netif_t *networkInterface = NULL;
// Structure for handle state of WiFi connection
static EventGroupHandle_t wifiState = NULL;

void wifiCallback(void *context, esp_event_base_t eventBase, int32_t eventId, void *eventData){

    // Turn of unused parameter warnings -> this function must has this signature
    (void)context;
    (void)eventData;
    // Event of starting WiFi and connecting to the WiFi
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_START) {
        ESP_LOGI(WIFI_LOG, "LOG: WiFi connecting to SSID: %s", WIFI_SSID);
        esp_wifi_connect();
        return;
    }

    // Event of disconnection from WiFi
    if (eventBase == WIFI_EVENT && eventId == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(WIFI_LOG, "LOG: WiFi disconnected");
        return;
    }

    // Event of getting IP from connected WiFi
    if (eventBase == IP_EVENT && eventId == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(WIFI_LOG, "LOG: WiFi got IP");
        if (wifiState) {
            xEventGroupSetBits(wifiState, BIT0);
        }
        return;
    }
}

esp_err_t wifiInit(void){

    // Result of the function
    esp_err_t result = ESP_OK;

    ESP_LOGI(WIFI_LOG, "LOG: Initializing WiFi");

    // If wifiState does not exist, create new one
    if (wifiState == NULL) {
        wifiState = xEventGroupCreate();
        if (wifiState == NULL){
            ESP_LOGE(WIFI_LOG, "ERROR: Creating WiFi event group failed.");  
            return ESP_ERR_NO_MEM;
        }
    } else {
        xEventGroupClearBits(wifiState, BIT0);
    }

    // Initialize network interface
    result = esp_netif_init();
    if (result != ESP_OK && result != ESP_ERR_INVALID_STATE){
        ESP_LOGE(WIFI_LOG, "ERROR: Initializing network interface failed: %s", esp_err_to_name(result));
        return result;
    } 

    // Create loop for handling events
    result = esp_event_loop_create_default();
    if (result != ESP_OK && result != ESP_ERR_INVALID_STATE){
        ESP_LOGE(WIFI_LOG, "ERROR: Creating event loop failed: %s", esp_err_to_name(result));
        return result;
    }

    // If network interface does not exist, create him
    if (networkInterface == NULL) {
        networkInterface = esp_netif_create_default_wifi_sta();
        if (networkInterface == NULL){
            ESP_LOGE(WIFI_LOG, "ERROR: Creating default WiFi station interface failed.");
            return ESP_FAIL;
        } 
    }

    // Initialize WiFi with default configuration
    wifi_init_config_t wifiInitConfiguration = WIFI_INIT_CONFIG_DEFAULT();
    result = esp_wifi_init(&wifiInitConfiguration);
    if (result != ESP_OK && result != ESP_ERR_INVALID_STATE){
        ESP_LOGE(WIFI_LOG, "ERROR: Initializing WiFi failed: %s", esp_err_to_name(result));
        return result;
    }

    // Register event handlers for WiFi events
    result = esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiCallback, NULL);
    if (result != ESP_OK) {
        ESP_LOGE(WIFI_LOG, "ERROR: Registering WiFi event handler failed: %s", esp_err_to_name(result));
        return result;
    }
    // Register event handlers for IP events
    result = esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifiCallback, NULL);
    if (result != ESP_OK) {
        ESP_LOGE(WIFI_LOG, "ERROR: Registering IP event handler failed: %s", esp_err_to_name(result));
        return result;
    }

    // Structure for WiFi connection configuration
    wifi_config_t connectionConfiguration;
    memset(&connectionConfiguration, 0, sizeof(wifi_config_t));

    // Copy SSID and Password to the connection configuration
    strncpy((char *)connectionConfiguration.sta.ssid, WIFI_SSID, sizeof(connectionConfiguration.sta.ssid) - 1);
    strncpy((char *)connectionConfiguration.sta.password, WIFI_PASSWORD, sizeof(connectionConfiguration.sta.password) - 1);

    // Set WiFi mode to station
    result = esp_wifi_set_mode(WIFI_MODE_STA);
    if (result != ESP_OK) {
        ESP_LOGE(WIFI_LOG, "ERROR: Setting WiFi mode failed: %s", esp_err_to_name(result));
        return result;
    }

    // Set WiFi configuration for station interface
    result = esp_wifi_set_config(WIFI_IF_STA, &connectionConfiguration);
    if (result != ESP_OK) {
        ESP_LOGE(WIFI_LOG, "ERROR: Setting WiFi configuration failed: %s", esp_err_to_name(result));
        return result;
    }

    // Start WiFi driver
    result = esp_wifi_start();
    if (result != ESP_OK) {
        ESP_LOGE(WIFI_LOG, "ERROR: Starting WiFi driver failed: %s", esp_err_to_name(result));
        return result;
    }

    // Wait for connection event bits  
    EventBits_t bits = xEventGroupWaitBits( wifiState, BIT0, pdFALSE, pdTRUE, pdMS_TO_TICKS(WIFI_EVENT_BITS_TIMEOUT_MS));
    // Check if the connection was successful
    result = (bits & BIT0) ? ESP_OK : ESP_FAIL;

    if (result != ESP_OK){
        ESP_LOGE(WIFI_LOG, "ERROR: Connecting to WiFi failed.");
    } else {
        ESP_LOGI(WIFI_LOG, "LOG: Connected to WiFi SSID: %s", WIFI_SSID);
    }
    
    return result;
}
