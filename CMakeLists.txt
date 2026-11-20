# File: CMakeLists.txt
# Author: Martin Zůbek
# Date: 2025-12-17
# Description: Top CMake configuration file for ESP-IDF project

cmake_minimum_required(VERSION 3.16)

set(EXTRA_COMPONENT_DIRS "src")

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(imp_project)
