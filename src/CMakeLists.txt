# File: CMakeLists.txt
# Author: Martin Zůbek, 
# Date: 2025-12-17
# Description: CMake configuration file for ESP-IDF project

idf_component_register(
    SRCS
        "main.c"
        "led/led.c"
        "sensor/sensor.c"
        "nonvolatile/nonvolatile.c"
        "serial/serial.c"
        "mqtt_wifi/mqtt.c"
        "mqtt_wifi/wifi.c"
    INCLUDE_DIRS
        "."
        "led"
        "sensor"
        "nonvolatile"
        "serial"
        "mqtt_wifi"
    PRIV_REQUIRES
        esp_driver_gpio
        esp_driver_ledc
        esp_driver_i2c
        esp_driver_uart
        driver
        nvs_flash
        esp_netif
        esp_event
        esp_wifi
)