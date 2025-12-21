# Makefile for IMP project at BUT FIT
# Author: Martin Zůbek, 
# Date: 17-12-2025

# Bash for executing commands
SHELL := /bin/bash

# Path to ESP-IDF
IDF_PATH ?= $(HOME)/esp/esp-idf
# Project directory
PROJECT_DIR := $(CURDIR)

# Name of the zip with project files
ZIP_NAME = .zip
# Files and directories to include for zipping
ZIP_FILES = Makefile CMakeLists.txt src/ web/ .pdf

# Helper function to call idf.py
define IDF
	. $(IDF_PATH)/export.sh >/dev/null && cd $(PROJECT_DIR) && idf.py $(1)
endef

# make build - builds the project
build:
	$(call IDF,build)

# make run - runs the project (build, flash, monitor)
run:
	$(call IDF,build)
	$(call IDF,flash)
	$(call IDF,monitor)

# make web - starts the web docker container
web:
	cd web && docker compose up -d

# make web_down - stops the web docker container
web_down:
	cd web && docker compose down -v

# make clean - clean up builded files
clean:
	$(call IDF,clean)
	rm -fr build
	rm -fr sdkconfig
	rm -fr dependencies.lock
	rm -fr managed_components/

# make pack - creates a zip with project files
pack:
	zip -r $(ZIP_NAME) $(ZIP_FILES)


.PHONY: build run clean pack debug web web_down
