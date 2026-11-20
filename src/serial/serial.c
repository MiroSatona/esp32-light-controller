/**
 * @file serial.c
 * @author Martin Zůbek
 * @brief Source file for serial communication by UART.
 * @date 2025-12-17
 */
#include "serial.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "led/led.h"
#include "nonvolatile/nonvolatile.h"
#include "types/constants.h"
#include "driver/gpio.h"


// Current user configuration
static USER_CONFIG_T currentConfiguration = {0};

void serialPrint(char *text) {
    uart_write_bytes(UART_NUM_0, text, strlen(text));
}


esp_err_t serialInit(void) {

    // UART configuration structure
    uart_config_t uartConfiguration;
    memset(&uartConfiguration, 0, sizeof(uart_config_t));

    // Configure UART parameters
    uartConfiguration.baud_rate  = SERIAL_BAUD_RATE;
    uartConfiguration.data_bits  = UART_DATA_8_BITS;
    uartConfiguration.parity     = UART_PARITY_DISABLE;
    uartConfiguration.stop_bits  = UART_STOP_BITS_1;
    uartConfiguration.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;
    uartConfiguration.source_clk = UART_SCLK_DEFAULT;

    // Result of the initialization
    esp_err_t result = ESP_OK;

    // Configure UART parameters
    result = uart_param_config(UART_NUM_0, &uartConfiguration);
    if (result != ESP_OK) {
        ESP_LOGE(SERIAL_LOG, "ERROR: UART configuration failed:%s.", esp_err_to_name(result));
        return result;
    }

    // Install UART driver
    result = uart_driver_install(UART_NUM_0, SERIAL_BUFFER_SIZE * 2, 0, 0, NULL, 0);
    if (result != ESP_OK) {
        ESP_LOGE(SERIAL_LOG, "ERROR: UART driver installation failed:%s.", esp_err_to_name(result));
        return result;
    }

    ESP_LOGI(SERIAL_LOG, "LOG: Serial communication initialized successfully.");
    return ESP_OK;
}

void printHelp(void) {
    serialPrint(
        "Commands:\r\n"
        "  set tmin <value>       - set minimum threshold, <value> is float\r\n"
        "  set tmax <value>       - set maximum threshold, <value> is float\r\n"
        "  set debounce <value>   - set hysteresis debounce, <value> unsigned int with minimum 4\r\n"
        "  set time <value>       - set measurement interval (ms), <value> int with minimum 200\r\n"
        "  show                   - show current configuration\r\n"
        "  help                   - show this help message\r\n"
    );
}

void printShow(void) {
    char msg[256];
    snprintf(
        msg,
        sizeof(msg),
        "%s Current Configuration:\r\n"
        "  Minimum Threshold (tmin): %.2f\r\n"
        "  Maximum Threshold (tmax): %.2f\r\n"
        "  Hysteresis Debounce: %u\r\n"
        "  Measurement Interval (ms): %lu\r\n\r\n",
        SERIAL_MARK,
        currentConfiguration.minThreshold,
        currentConfiguration.maxThreshold,
        currentConfiguration.hysteresisCount,
        (unsigned long)currentConfiguration.timeForNextMeasurement
    );
    serialPrint(msg);
}

bool isValidValue(VALUE_TYPE_T type, float fValue, int iValue) {
    // Validate based on type
    char printBuffer[BUFFER_SIZE];
    switch (type) {
        // Minimum threshold
        case VALUE_TMIN:
            if (fValue >= currentConfiguration.maxThreshold) {
                snprintf(
                    printBuffer,
                    sizeof(printBuffer),
                    "%s ERROR: Tmin must be lower than tmax (tmax=%.2f)\r\n",
                    SERIAL_MARK,
                    currentConfiguration.maxThreshold
                );
                serialPrint(printBuffer);
                return false;
            }
            return true;

        // Maximum threshold
        case VALUE_TMAX:
            if (fValue <= currentConfiguration.minThreshold) {
                snprintf(
                    printBuffer,
                    sizeof(printBuffer),
                    "%s ERROR: Tmax must be higher than tmin (tmin=%.2f)\r\n",
                    SERIAL_MARK,
                    currentConfiguration.minThreshold
                );
                serialPrint(printBuffer);
                return false;
            }
            return true;

        // Hysteresis debounce
        case VALUE_DEBOUNCE:
            if (iValue < HYSTERESIS_COUNT_MIN) {
                snprintf(
                    printBuffer,
                    sizeof(printBuffer),
                    "%s ERROR: Debounce must be >= %d\r\n",
                    SERIAL_MARK,
                    HYSTERESIS_COUNT_MIN
                );
                serialPrint(printBuffer);
                return false;
            }
            return true;

        // Measurement interval
        case VALUE_TIME:
            if (iValue < DEFAULT_TIME_FOR_NEXT_MEASUREMENT) {
                snprintf(
                    printBuffer,
                    sizeof(printBuffer),
                    "%s ERROR: Measurement interval must be >= %d ms\r\n",
                    SERIAL_MARK,
                    DEFAULT_TIME_FOR_NEXT_MEASUREMENT
                );
                serialPrint(printBuffer);
                return false;
            }
            return true;

        // Unknown type
        default:
            snprintf(printBuffer, sizeof(printBuffer), "%s ERROR: Unknown value type\r\n", SERIAL_MARK);
            serialPrint(printBuffer);
            return false;
    }
}

void serialTask(void) {

    // Buffer for storing incoming data
    char buffer[BUFFER_SIZE];

    // Load current configuration from non-volatile storage
    esp_err_t result = getNonvolatileData(&currentConfiguration);
    if (result != ESP_OK) return;

    // Store current configuration to LED module
    setMinThreshold(currentConfiguration.minThreshold);
    setMaxThreshold(currentConfiguration.maxThreshold);
    setHysteresisSamples(currentConfiguration.hysteresisCount);
    setTimeForNextMeasurement(currentConfiguration.timeForNextMeasurement);

    // Get the following serial mark to the new line
    serialPrint("\r\n");
    // Print prompt mark
    serialPrint(SERIAL_MARK);
    // Current line length
    int lineLength = 0;
    // Clear buffer
    memset(buffer, 0, sizeof(buffer));

    while (true) {
        // Readed character
        uint8_t character = 0;
        // Count of readed bytes
        int bytesRead = uart_read_bytes(UART_NUM_0, &character, 1, pdMS_TO_TICKS(READ_TIMEOUT_MS));
        // If no bytes were read, use delay and continue
        if (bytesRead <= 0) {
            vTaskDelay(pdMS_TO_TICKS(RESTART_READ_DELAY_MS));
            continue;
        }

        // For enter press
        if (character == '\r' || character == '\n') {
            // Go to new line
            uart_write_bytes(UART_NUM_0, "\r\n", 2);
            buffer[lineLength] = '\0';
            // Empty line, just go to next line
            if (lineLength == 0) {
                serialPrint(SERIAL_MARK);
                continue;
            }

            // For storing float value
            float floatValue = 0.0f;
            // For storing integer value
            int integerValue = 0;
            // Flag to print prompt mark
            bool printMark = false;

            // Process treshold minimum command
            if (sscanf(buffer, "set tmin %f", &floatValue) == 1) {
                if (isValidValue(VALUE_TMIN, floatValue, 0)) {
                    currentConfiguration.minThreshold = floatValue;
                    setMinThreshold(floatValue);
                
                    char printBuffer[BUFFER_SIZE];
                    snprintf(
                        printBuffer,
                        sizeof(printBuffer),
                        "%s Minimum threshold was set to %.2f successfully.\r\n",
                        SERIAL_MARK,
                        floatValue
                    );
                    serialPrint(printBuffer);
                
                    nonvolatileSave(&currentConfiguration);
                } 
                printMark = true;
            }

            // Process treshold maximum command
            else if (sscanf(buffer, "set tmax %f", &floatValue) == 1) {
                if (isValidValue(VALUE_TMAX, floatValue, 0)) {
                    currentConfiguration.maxThreshold = floatValue;
                    setMaxThreshold(floatValue);
                    
                    char printBuffer[BUFFER_SIZE];
                    snprintf(
                        printBuffer,
                        sizeof(printBuffer),
                        "%s Maximum threshold was set to %.2f successfully.\r\n",
                        SERIAL_MARK,
                        floatValue
                    );
                    serialPrint(printBuffer);

                    nonvolatileSave(&currentConfiguration);
                } 
                printMark = true;
            }

            // Process debounce command
            else if (sscanf(buffer, "set debounce %d", &integerValue) == 1) {
                if (isValidValue(VALUE_DEBOUNCE, 0.0f, integerValue)) {
                    currentConfiguration.hysteresisCount = (unsigned int)integerValue;
                    setHysteresisSamples((unsigned int)integerValue);
                    char printBuffer[BUFFER_SIZE];
                    snprintf(
                        printBuffer,
                        sizeof(printBuffer),
                        "%s Debounce set to %d successfully.\r\n",
                        SERIAL_MARK,
                        integerValue
                    );
                    serialPrint(printBuffer);
                    nonvolatileSave(&currentConfiguration);
                } 
                printMark = true;
            }

            // Process measurement interval command
            else if (sscanf(buffer, "set time %d", &integerValue) == 1) {
                if (isValidValue(VALUE_TIME, 0.0f, integerValue)) {
                    currentConfiguration.timeForNextMeasurement = integerValue;
                    setTimeForNextMeasurement(integerValue);
                    
                    char printBuffer[BUFFER_SIZE];
                    snprintf(
                        printBuffer,
                        sizeof(printBuffer),
                        "%s Measurement interval set to %d ms successfully.\r\n",
                        SERIAL_MARK,
                        integerValue
                    );
                    serialPrint(printBuffer);
                    
                    nonvolatileSave(&currentConfiguration);
                } 
                printMark = true;
            }

            // Process show command
            else if (strncmp(buffer, "show", 4) == 0) {
                printShow();
                printMark = true;
            }

            // Process help command
            else if (strncmp(buffer, "help", 4) == 0) {
                printHelp();
                printMark = true;
            }
            
            // Unknown command
            else {
                char printBuffer[96];
                snprintf(
                    printBuffer,
                    sizeof(printBuffer),
                    "%s ERROR: Unknown command. Use 'help'.\r\n",
                    SERIAL_MARK
                );
                serialPrint(printBuffer);
                printMark = true;
            }

            // Reset line length and buffer
            lineLength = 0;
            memset(buffer, 0, sizeof(buffer));

            // Print prompt mark if needed
            if (printMark) {
                serialPrint(SERIAL_MARK);
            }

            continue;
        }

        // Handle backspace or delete
        if (character == ASCII_DELETE || character == ASCII_BACKSPACE) {
            if (lineLength > 0) {
                lineLength--;
                buffer[lineLength] = '\0';
                uart_write_bytes(UART_NUM_0, "\b \b", 3);
            }
            continue;
        }

        // Store character in buffer if there is space
        if (lineLength < (BUFFER_SIZE - 1)) {
            buffer[lineLength++] = (char)character;
            uart_write_bytes(UART_NUM_0, (const char *)&character, 1);
        }
    }
}
