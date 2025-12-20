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

static USER_CONFIG_T currentConfiguration = {0};

static void serialPrint(const char *fmt, ...) {
    char out[512];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(out, sizeof(out), fmt, args);
    va_end(args);
    if (n < 0) return;
    if (n > (int)sizeof(out)) n = (int)sizeof(out);
    uart_write_bytes(SERIAL_UART_NUM, out, n);
}

esp_err_t serialInit(void) {
    uart_config_t uartConfiguration;
    memset(&uartConfiguration, 0, sizeof(uart_config_t));

    uartConfiguration.baud_rate  = SERIAL_BAUD_RATE;
    uartConfiguration.data_bits  = UART_DATA_8_BITS;
    uartConfiguration.parity     = UART_PARITY_DISABLE;
    uartConfiguration.stop_bits  = UART_STOP_BITS_1;
    uartConfiguration.flow_ctrl  = UART_HW_FLOWCTRL_DISABLE;
    uartConfiguration.source_clk = UART_SCLK_DEFAULT;

    esp_err_t result;

    result = uart_param_config(SERIAL_UART_NUM, &uartConfiguration);
    if (result != ESP_OK) {
        ESP_LOGE(SERIAL_LOG, "ERROR: UART configuration failed:%s.", esp_err_to_name(result));
        return result;
    }

    if (SERIAL_UART_NUM != UART_NUM_0) {
        result = uart_set_pin(
            SERIAL_UART_NUM,
            SERIAL_TX_PIN,
            SERIAL_RX_PIN,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        );
        if (result != ESP_OK) {
            ESP_LOGE(SERIAL_LOG, "ERROR: UART set pin failed:%s.", esp_err_to_name(result));
            return result;
        }
    }

    result = uart_driver_install(
        SERIAL_UART_NUM,
        SERIAL_BUFFER_SIZE * 2,
        0,
        0,
        NULL,
        0
    );
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
    serialPrint(
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
}

bool isValidValue(VALUE_TYPE_T type, float fValue, int iValue) {
    switch (type) {
        case VALUE_TMIN:
            if (fValue >= currentConfiguration.maxThreshold) {
                serialPrint(
                    "%s ERROR: tmin must be lower than tmax (tmax=%.2f)\r\n",
                    SERIAL_MARK,
                    currentConfiguration.maxThreshold
                );
                return false;
            }
            return true;

        case VALUE_TMAX:
            if (fValue <= currentConfiguration.minThreshold) {
                serialPrint(
                    "%s ERROR: tmax must be higher than tmin (tmin=%.2f)\r\n",
                    SERIAL_MARK,
                    currentConfiguration.minThreshold
                );
                return false;
            }
            return true;

        case VALUE_DEBOUNCE:
            if (iValue < HYSTERESIS_COUNT_MIN) {
                serialPrint(
                    "%s ERROR: debounce must be >= %d\r\n",
                    SERIAL_MARK,
                    HYSTERESIS_COUNT_MIN
                );
                return false;
            }
            return true;

        case VALUE_TIME:
            if (iValue < DEFAULT_TIME_FOR_NEXT_MEASUREMENT) {
                serialPrint(
                    "%s ERROR: measurement interval must be >= %d ms\r\n",
                    SERIAL_MARK,
                    DEFAULT_TIME_FOR_NEXT_MEASUREMENT
                );
                return false;
            }
            return true;

        default:
            serialPrint("%s ERROR: Unknown value type\r\n", SERIAL_MARK);
            return false;
    }
}

void serialTask(void) {
    char buffer[SERIAL_BUFFER_SIZE];

    esp_err_t result = getNonvolatileData(&currentConfiguration);
    if (result != ESP_OK) return;

    setMinThreshold(currentConfiguration.minThreshold);
    setMaxThreshold(currentConfiguration.maxThreshold);
    setHysteresisSamples(currentConfiguration.hysteresisCount);
    setTimeForNextMeasurement(currentConfiguration.timeForNextMeasurement);

    serialPrint("%s", SERIAL_MARK);

    int lineLen = 0;
    memset(buffer, 0, sizeof(buffer));

    while (true) {
        uint8_t ch = 0;
        int n = uart_read_bytes(SERIAL_UART_NUM, &ch, 1, pdMS_TO_TICKS(100));
        if (n <= 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        if (ch == '\r' || ch == '\n') {
            uart_write_bytes(SERIAL_UART_NUM, "\r\n", 2);
            buffer[lineLen] = '\0';

            if (lineLen == 0) {
                serialPrint("%s", SERIAL_MARK);
                continue;
            }

            float floatValue = 0.0f;
            int integerValue = 0;
            bool printPromptAgain = false;

            if (sscanf(buffer, "set tmin %f", &floatValue) == 1) {
                if (isValidValue(VALUE_TMIN, floatValue, 0)) {
                    currentConfiguration.minThreshold = floatValue;
                    setMinThreshold(floatValue);
                    serialPrint(
                        "%s Minimum threshold was set to %.2f successfully.\r\n",
                        SERIAL_MARK,
                        floatValue
                    );
                    nonvolatileSave(&currentConfiguration);
                } else {
                    serialPrint(
                        "%s WARNING: Invalid minimum threshold value.\r\n",
                        SERIAL_MARK
                    );
                }
                printPromptAgain = true;
            }

            else if (sscanf(buffer, "set tmax %f", &floatValue) == 1) {
                if (isValidValue(VALUE_TMAX, floatValue, 0)) {
                    currentConfiguration.maxThreshold = floatValue;
                    setMaxThreshold(floatValue);
                    serialPrint(
                        "%s Maximum threshold was set to %.2f successfully.\r\n",
                        SERIAL_MARK,
                        floatValue
                    );
                    nonvolatileSave(&currentConfiguration);
                } else {
                    serialPrint(
                        "%s WARNING: Invalid maximum threshold value.\r\n",
                        SERIAL_MARK
                    );
                }
                printPromptAgain = true;
            }

            else if (sscanf(buffer, "set debounce %d", &integerValue) == 1) {
                if (isValidValue(VALUE_DEBOUNCE, 0.0f, integerValue)) {
                    currentConfiguration.hysteresisCount = (unsigned int)integerValue;
                    setHysteresisSamples((unsigned int)integerValue);
                    serialPrint(
                        "%s Debounce set to %d successfully.\r\n",
                        SERIAL_MARK,
                        integerValue
                    );
                    nonvolatileSave(&currentConfiguration);
                } else {
                    serialPrint(
                        "%s WARNING: Invalid debounce value.\r\n",
                        SERIAL_MARK
                    );
                }
                printPromptAgain = true;
            }

            else if (sscanf(buffer, "set time %d", &integerValue) == 1) {
                if (isValidValue(VALUE_TIME, 0.0f, integerValue)) {
                    currentConfiguration.timeForNextMeasurement = integerValue;
                    setTimeForNextMeasurement(integerValue);
                    serialPrint(
                        "%s Measurement interval set to %d ms successfully.\r\n",
                        SERIAL_MARK,
                        integerValue
                    );
                    nonvolatileSave(&currentConfiguration);
                } else {
                    serialPrint(
                        "%s WARNING: Invalid measurement interval.\r\n",
                        SERIAL_MARK
                    );
                }
                printPromptAgain = true;
            }

            else if (strncmp(buffer, "show", 4) == 0) {
                printShow();
                printPromptAgain = true;
            }

            else if (strncmp(buffer, "help", 4) == 0) {
                printHelp();
                printPromptAgain = true;
            }

            else {
                serialPrint(
                    "%s ERROR: Unknown command. Use 'help'.\r\n",
                    SERIAL_MARK
                );
                printPromptAgain = true;
            }

            lineLen = 0;
            memset(buffer, 0, sizeof(buffer));

            if (printPromptAgain) {
                serialPrint("%s", SERIAL_MARK);
            }

            continue;
        }

        if (ch == 0x7F || ch == 0x08) {
            if (lineLen > 0) {
                lineLen--;
                buffer[lineLen] = '\0';
                uart_write_bytes(SERIAL_UART_NUM, "\b \b", 3);
            }
            continue;
        }

        if (lineLen < (SERIAL_BUFFER_SIZE - 1)) {
            buffer[lineLen++] = (char)ch;
            uart_write_bytes(SERIAL_UART_NUM, (const char *)&ch, 1);
        }
    }
}
