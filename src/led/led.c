/**
 * @file led.c
 * @author Martin Zůbek, 
 * @brief Source file for LED init and handle brightness independent on threshold or without threshold.
 * @date 2025-12-16
 */

#include "led.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "esp_err.h"
#include "esp_log.h"
#include <stdbool.h>
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "types/constants.h"


// Minimum treshold for LED with threshold
static float MIN_THRESHOLD = LUX_MIN;
// Maximum treshold for LED with threshold
static float MAX_THRESHOLD = LUX_MAX;
// Flag for hysteresis state
static bool hysteresis = false;

esp_err_t ledsInit(void){

    // Variable for storing result of the configuration functions
    esp_err_t result = ESP_OK;

    // Create configuration of timer for both LEDs
    ledc_timer_config_t ledTimer;
    memset(&ledTimer, 0, sizeof(ledc_timer_config_t));

    // Set timer parameters for PWM
    ledTimer.speed_mode      = LEDC_LOW_SPEED_MODE;
    ledTimer.duty_resolution = LEDC_TIMER_13_BIT;
    ledTimer.timer_num       = LEDC_TIMER_0;
    ledTimer.freq_hz         = LED_FREQUENCY;
    ledTimer.clk_cfg         = LEDC_AUTO_CLK;

    // Configure timer
    result = ledc_timer_config(&ledTimer);
    // Check for errors
    if (result != ESP_OK) {
        ESP_LOGE(LED_LOG, "ERROR: ledc_timer_config failed: %s.", esp_err_to_name(result));
        return result;
    }

    // Create configuration for channel of LED without threshold
    ledc_channel_config_t ledChanelConfig;
    memset(&ledChanelConfig, 0, sizeof(ledc_channel_config_t));

    // Set channel parameters
    ledChanelConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    ledChanelConfig.channel    = LED_CHANNEL;
    ledChanelConfig.timer_sel  = LEDC_TIMER_0;
    ledChanelConfig.gpio_num   = LED_GPIO;
    ledChanelConfig.duty       = 0;
    ledChanelConfig.hpoint     = 0;

    // Configure channel
    result = ledc_channel_config(&ledChanelConfig);
    // Check for errors
    if (result != ESP_OK) {
        ESP_LOGE(LED_LOG, "ERROR: Configuration of the channel for LED without threshold failed: %s.", esp_err_to_name(result));
        return result;
    }

    // Create configuration for channel of LED with threshold
    ledc_channel_config_t ledChanelThreshConfig;
    memset(&ledChanelThreshConfig, 0, sizeof(ledc_channel_config_t));

    // Set channel parameters
    ledChanelThreshConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    ledChanelThreshConfig.channel    = LED_THRESH_CHANNEL;
    ledChanelThreshConfig.timer_sel  = LEDC_TIMER_0;
    ledChanelThreshConfig.gpio_num   = LED_THRESH_GPIO;
    ledChanelThreshConfig.duty       = 0;
    ledChanelThreshConfig.hpoint     = 0;

    // Configure channel
    result = ledc_channel_config(&ledChanelThreshConfig);
    // Check for errors
    if (result != ESP_OK) {
        ESP_LOGE(LED_LOG, "ERROR: Configuration of the channel for LED with threshold failed: %s.", esp_err_to_name(result));
        return result;
    }

    return ESP_OK;
}

 esp_err_t setBrightnessWithNoThreshold(uint32_t brightness){

    // Variable for storing result of setting duty and updating duty on LED
    esp_err_t result = ESP_OK;

    // If brightness is higher than maximum lux, turn off the LED
    if (brightness >= LUX_MAX) {
        // Set duty to minimum value
        result = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL, LED_PWM_MIN);
        // Check the result
        if (result != ESP_OK) {
            ESP_LOGE(LED_LOG, "ERROR: Setting duty for LED without threshold failed: %s.", esp_err_to_name(result));
            return result;
        }
        // Update the duty
        result = ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL);
        // Check the result
        if (result != ESP_OK) {
            ESP_LOGE(LED_LOG, "ERROR: Updating duty for LED without threshold failed: %s.", esp_err_to_name(result));
            return result;
        }

        return result;
    }

    // If brightness is lower than minimum lux, set LED to maximum brightness
    if (brightness <= LUX_MIN) {
        // Set duty to maximum value
        result = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL, LED_PWM_MAX);
        // Check the result
        if (result != ESP_OK) {
            ESP_LOGE(LED_LOG, "ERROR: Setting duty for LED without threshold failed: %s.", esp_err_to_name(result));
            return result;
        }
        // Update the duty
        result = ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL);
        // Check the result
        if (result != ESP_OK) {
            ESP_LOGE(LED_LOG, "ERROR: Updating duty for LED without threshold failed: %s.", esp_err_to_name(result));
            return result;
        }

        return result;
    }

    // Normalize brightness value and invert it
    float normalized =  1.0f - ((float)brightness / LUX_MAX);
    // Apply gamma correction for human eye
    float gammaCorrected = powf(normalized, GAMMA_REDUCTION);
    // Calculate duty
    uint32_t duty = (uint32_t)(gammaCorrected * LED_PWM_MAX);

    // Set calculated duty for LED
    result = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL, duty);
    // Check the result
    if (result != ESP_OK) {
        ESP_LOGE(LED_LOG, "ERROR: Setting duty for LED without threshold failed: %s.", esp_err_to_name(result));
        return result;
    }
    // Update the duty
    result = ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL);
    // Check the result
    if (result != ESP_OK) {
        ESP_LOGE(LED_LOG, "ERROR: Updating duty for LED without threshold failed: %s.", esp_err_to_name(result));
        return result;
    }

    return result;
}


 esp_err_t setBrightnessWithThreshold(uint32_t brightness){
    
    // Variable for storing result of setting duty and updating duty on LED
    esp_err_t result = ESP_OK;

    // Hystersis is in the off state
    if (!hysteresis) {
        // If brightness is lower than minimum threshold, set hysteresis to on state
        if ((float)brightness <= MIN_THRESHOLD) {
            hysteresis = true;
        // If brightness is higher than maximum threshold, set LED to minimum brightness (OFF)
        } else {
            result = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_THRESH_CHANNEL, LED_PWM_MIN);
            if (result != ESP_OK) {
                ESP_LOGE(LED_LOG, "ERROR: Setting duty for LED with threshold failed: %s.", esp_err_to_name(result));
                return result;
            }
            
            result = ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_THRESH_CHANNEL);
            if (result != ESP_OK) {
                ESP_LOGE(LED_LOG, "ERROR: Updating duty for LED with threshold failed: %s.", esp_err_to_name(result));
                return result;
            }
            return result;
        }
    // Hystersis is in the on state
    } else {
        // If brightness is higher than maximum threshold, set hysteresis to off state
        if ((float)brightness >= MAX_THRESHOLD) {
            hysteresis = false; 
            result = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_THRESH_CHANNEL, LED_PWM_MIN);
            if (result != ESP_OK) {
                ESP_LOGE(LED_LOG, "ERROR: Setting duty for LED with threshold failed: %s.", esp_err_to_name(result));
                return result;
            }
            result = ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_THRESH_CHANNEL);
            if (result != ESP_OK) {
                ESP_LOGE(LED_LOG, "ERROR: Updating duty for LED with threshold failed: %s.", esp_err_to_name(result));
                return result;
            }
            return result;
        }
    }

    // Calculate threshold interval
    float thresholdInterval = (MAX_THRESHOLD - MIN_THRESHOLD);
    // If threshold interval is too small, set LED to minimum brightness (OFF)
    if (thresholdInterval <= 0.0001f) {
        result = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_THRESH_CHANNEL, LED_PWM_MIN);
        if (result != ESP_OK) {
            ESP_LOGE(LED_LOG, "ERROR: Setting duty for LED with threshold failed: %s.", esp_err_to_name(result));
            return result;
        }
        result = ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_THRESH_CHANNEL);
        if (result != ESP_OK) {
            ESP_LOGE(LED_LOG, "ERROR: Updating duty for LED with threshold failed: %s.", esp_err_to_name(result));
            return result;
        }

        return result;
    }

    // Normalize brightness value within threshold interval and invert it
    float normalized = HUNDRED_PERCENTAGE - ((float)brightness - MIN_THRESHOLD) / thresholdInterval;

    // Check normalized value
    if (normalized > HUNDRED_PERCENTAGE) normalized = HUNDRED_PERCENTAGE;
    if (normalized < ZERO_PERCENTAGE) normalized = ZERO_PERCENTAGE;

    // Apply gamma correction for human eye
    float gammaCorrected = powf(normalized, GAMMA_REDUCTION);
    // Calculate duty
    uint32_t duty = (uint32_t)(gammaCorrected * LED_PWM_MAX);

    // Set calculated duty for LED
    result = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_THRESH_CHANNEL, duty);
    if (result != ESP_OK) {
        ESP_LOGE(LED_LOG, "ERROR: Setting duty for LED with threshold failed: %s.", esp_err_to_name(result));
        return result;
    }
    
    // Update the duty
    result = ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_THRESH_CHANNEL);
    if (result != ESP_OK) {
        ESP_LOGE(LED_LOG, "ERROR: Updating duty for LED with threshold failed: %s.", esp_err_to_name(result));
        return result;
    }

    return result;
}

void setMinThreshold(float minThreshold){
    MIN_THRESHOLD = minThreshold;
}

void setMaxThreshold(float maxThreshold){
    MAX_THRESHOLD = maxThreshold;
}

float getMinThreshold(void){
    return MIN_THRESHOLD;
}

float getMaxThreshold(void){
    return MAX_THRESHOLD;
}

bool getHysteresisState(void){
    return hysteresis;
}