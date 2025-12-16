SHELL := /bin/bash

IDF_PATH ?= $(HOME)/esp/esp-idf
PROJECT_DIR := $(CURDIR)

define IDF
	. $(IDF_PATH)/export.sh >/dev/null && cd $(PROJECT_DIR) && idf.py $(1)
endef

build:
	$(call IDF,build)

run:
	$(call IDF,build)
	$(call IDF,flash monitor)

clean:
	$(call IDF,clean)
	rm -fr sdkconfig

.PHONY: build run clean
