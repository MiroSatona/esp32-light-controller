/**
 * @file led.c
 * @author Martin Zůbek, 
 * @brief Source file for LEDs init and handle brightness and handle thresholds.
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
static float MIN_THRESHOLD = DEFAULT_LUX_MIN;
// Maximum treshold for LED with threshold
static float MAX_THRESHOLD = DEFAULT_LUX_MAX;
// Number of samples for hysteresis debounce
static unsigned int HYSTERESIS_SAMPLES = HYSTERESIS_COUNT_MIN;
// Flag for hysteresis state above
static bool HYSTERESIS_STATE_ABOVE = false;
// Flag for hysteresis state bellow
static bool HYSTERESIS_STATE_BELLOW = false;
// Value for hysteresis samples
static unsigned int HYSTERESIS_COUNTER_ABOVE = 0;
// Counter for hysteresis above maximum threshold
static unsigned int HYSTERESIS_COUNTER_BELLOW = 0;

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

    // Create configuration for channel of LED on PIN 25
    ledc_channel_config_t ledChanelConfig;
    memset(&ledChanelConfig, 0, sizeof(ledc_channel_config_t));

    // Set channel parameters
    ledChanelConfig.speed_mode = LEDC_LOW_SPEED_MODE;
    ledChanelConfig.channel    = LED_CHANNEL_25;
    ledChanelConfig.timer_sel  = LEDC_TIMER_0;
    ledChanelConfig.gpio_num   = LED_GPIO_PIN_25;
    ledChanelConfig.duty       = 0;
    ledChanelConfig.hpoint     = 0;

    // Configure channel
    result = ledc_channel_config(&ledChanelConfig);
    // Check for errors
    if (result != ESP_OK) {
        ESP_LOGE(LED_LOG, "ERROR: Configuration of the channel on PIN 25 failed: %s.", esp_err_to_name(result));
        return result;
    }

    // Change setting of channel configuration structure for LED on PIN 26
    ledChanelConfig.gpio_num   = LED_GPIO_PIN_26;
    ledChanelConfig.channel = LED_CHANNEL_26; 

    // Configure channel
    result = ledc_channel_config(&ledChanelConfig);
    // Check for errors
    if (result != ESP_OK) {
        ESP_LOGE(LED_LOG, "ERROR: Configuration of the channel for LED on PIN 26 failed: %s.", esp_err_to_name(result));
        return result;
    }

    return ESP_OK;
}

esp_err_t setDuty(uint32_t duty){

    // Result of the function
    esp_err_t result = ESP_OK;

    /**
     * ==================================================================================================
     *                            SET DUTY FOR LEDS ON PIN 25 AND PIN 26
     * ================================================================================================== 
     */

    result  = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL_25, duty);
    if (result != ESP_OK){
        ESP_LOGE(LED_LOG, "ERROR: Setting duty for LED on PIN 25 failed: %s.", esp_err_to_name(result));
        return result;
    }

    result = ledc_set_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL_26, duty);
    if (result != ESP_OK){
        ESP_LOGE(LED_LOG, "ERROR: Setting duty for LED on PIN 26 failed: %s.", esp_err_to_name(result));
        return result;
    }

    /**
     * ==================================================================================================
     *                             UPDATE DUTY FOR LEDS ON PIN 25 AND PIN 26
     * ================================================================================================== 
     */

    result = ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL_25);
    if (result != ESP_OK){
        ESP_LOGE(LED_LOG, "ERROR: Updating duty for LED on PIN 25 failed: %s.", esp_err_to_name(result));
        return result;
    }

    result = ledc_update_duty(LEDC_LOW_SPEED_MODE, LED_CHANNEL_26);
    if (result != ESP_OK){
        ESP_LOGE(LED_LOG, "ERROR: Updating duty for LED on PIN 26 failed: %s.", esp_err_to_name(result));
        return result;
    }

    return result;
}
 



esp_err_t setBrightness(float luxValue){   

    // Result of the function
    esp_err_t result = ESP_OK;

    // Calculate the interval between maximum and minimum threshold
    float interval = (MAX_THRESHOLD - MIN_THRESHOLD);

    // If the interval is too small, turn off the LEDs and reset hysteresis states and counters
    if (interval <= TOO_SMALL_INTERVAL) {
        HYSTERESIS_STATE_ABOVE = false;
        HYSTERESIS_STATE_BELLOW = false;
        HYSTERESIS_COUNTER_ABOVE = 0;
        HYSTERESIS_COUNTER_BELLOW = 0;
        return setDuty(LED_PWM_MIN);
    }

    // Enter to HYSTERESIS BELLOW state 
    if (luxValue <= MIN_THRESHOLD && !HYSTERESIS_STATE_BELLOW) {
        HYSTERESIS_STATE_BELLOW = true;
        HYSTERESIS_COUNTER_BELLOW = 0;
        HYSTERESIS_STATE_ABOVE = false;
        HYSTERESIS_COUNTER_ABOVE = 0;
    }

    // Enter to HYSTERESIS ABOVE state
    if (luxValue >= MAX_THRESHOLD && !HYSTERESIS_STATE_ABOVE) {
        HYSTERESIS_STATE_ABOVE = true;
        HYSTERESIS_COUNTER_ABOVE = 0;
        HYSTERESIS_STATE_BELLOW = false;
        HYSTERESIS_COUNTER_BELLOW = 0;
    }

    // Handle HYSTERESIS ABOVE state
    if (HYSTERESIS_STATE_ABOVE) {
        if (luxValue >= MAX_THRESHOLD) {
            if (HYSTERESIS_COUNTER_ABOVE < HYSTERESIS_SAMPLES) {
                HYSTERESIS_COUNTER_ABOVE++;
            }
        } else {
            HYSTERESIS_COUNTER_ABOVE = 0;
            HYSTERESIS_STATE_ABOVE = false;
        }
    }

    // Handle HYSTERESIS BELLOW state
    if (HYSTERESIS_STATE_BELLOW) {
        if (luxValue <= MIN_THRESHOLD) {
            if (HYSTERESIS_COUNTER_BELLOW < HYSTERESIS_SAMPLES) {
                HYSTERESIS_COUNTER_BELLOW++;
            }
        } else {
            HYSTERESIS_COUNTER_BELLOW = 0;
            HYSTERESIS_STATE_BELLOW = false;
        }
    }

    // Check if in HYSTERESIS ABOVE state and counter reached samples
    if (HYSTERESIS_STATE_ABOVE && HYSTERESIS_COUNTER_ABOVE >= HYSTERESIS_SAMPLES) {
        HYSTERESIS_STATE_ABOVE = false;
        HYSTERESIS_COUNTER_ABOVE = 0;
        HYSTERESIS_STATE_BELLOW = false;
        HYSTERESIS_COUNTER_BELLOW = 0;
        return setDuty(LED_PWM_MIN);
    }

    // Check if in HYSTERESIS BELLOW state and counter reached samples
    if (HYSTERESIS_STATE_BELLOW && HYSTERESIS_COUNTER_BELLOW >= HYSTERESIS_SAMPLES) {
        HYSTERESIS_STATE_BELLOW = false;
        HYSTERESIS_COUNTER_BELLOW = 0;
        HYSTERESIS_STATE_ABOVE = false;
        HYSTERESIS_COUNTER_ABOVE = 0;
        return setDuty(LED_PWM_MAX);
    }

    if(HYSTERESIS_STATE_ABOVE || HYSTERESIS_STATE_BELLOW) return ESP_OK;


    // Normalize brightness value within threshold interval and invert it
    float normalized = HUNDRED_PERCENTAGE - (luxValue - MIN_THRESHOLD) / interval;
    // Apply gamma correction for human eye
    float gammaCorrected = powf(normalized, GAMMA_REDUCTION);
    // Calculate duty
    uint32_t duty = (uint32_t)(gammaCorrected * LED_PWM_MAX);

    // Set duty for both LEDS
    result = setDuty(duty);

    if(result == ESP_OK) ESP_LOGI(LED_LOG, "Set duty to %u for lux value %.2f was successful for both LEDS!", duty,  luxValue);
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

bool getHysteresisStateAbove(void){
    return HYSTERESIS_STATE_ABOVE;
}

void setHysteresisStateAbove(bool state){
    HYSTERESIS_STATE_ABOVE = state;
}

bool getHysteresisStateBellow(void){
    return HYSTERESIS_STATE_BELLOW;
}

void setHysteresisStateBellow(bool state){
    HYSTERESIS_STATE_BELLOW = state;
}

unsigned int getHysteresisSamples(void){
    return HYSTERESIS_SAMPLES;
}

void setHysteresisSamples(unsigned int samples){
    HYSTERESIS_SAMPLES = samples;
}

