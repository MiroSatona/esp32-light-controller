SHELL := /bin/bash

IDF_PATH ?= $(HOME)/esp/esp-idf
PROJECT_DIR := $(CURDIR)

ZIP_NAME = .zip
ZIP_FILES = src/ CMakeLists.txt Makefile .pdf

define IDF
	. $(IDF_PATH)/export.sh >/dev/null && cd $(PROJECT_DIR) && idf.py $(1)
endef

build:
	$(call IDF,build)

run:
	$(call IDF,build)
	$(call IDF,flash)
	$(call IDF,monitor --print-filter "*:E")

clean:
	$(call IDF,clean)
	rm -fr build
	rm -fr sdkconfig

pack:
	zip -r $(ZIP_NAME) $(ZIP_FILES)

.PHONY: build run clean pack debug