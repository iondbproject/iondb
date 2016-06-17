################################################################################
###	Makefile wrapper for common repository actions.
###	Author: Graeme Douglas
###	
###	Copyright 2016 The University of British Columbia, Graeme Douglas
###	Licensed under the Apache License, Version 2.0 (the "License");
###	you may not use this file except in compliance with the License.
###	You may obtain a copy of the License at
###	
###		http://www.apache.org/licenses/LICENSE-2.0
###	
###	Unless required by applicable law or agreed to in writing,
###	software distributed under the License is distributed on an
###	"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
###	either express or implied. See the License for the specific
###	language governing permissions and limitations under the
###	License.
################################################################################

## Configurables ###############################################################
# External commands.
MKDIR := mkdir -p
RM    := rm -rf
CP    := cp
PY    := python

SRC          := src
BUILD_DIR    := build
TEST_BIN_DIR := $(BUILD_DIR)/bin
DOTFILES_DIR := dotfiles
SCRIPTS_DIR  := $(DOTFILES_DIR)/scripts
################################################################################

## Targets/Actions #############################################################
.PHONY: init_dirs
init_dirs:
	$(MKDIR) $(BUILD_DIR)
	$(MKDIR) $(TEST_BIN_DIR)

.PHONY: clean
clean: init_dirs
	$(RM) $(BUILD_DIR)/*

$(BUILD_DIR)/Makefile: init_dirs
	cd $(BUILD_DIR) ; cmake ..

.PHONY: all
all: init_dirs $(BUILD_DIR)/Makefile
	cd $(BUILD_DIR) ; make all

.PHONY: test
test: init_dirs $(SCRIPTS_DIR)/runalltests.py
	cd $(TEST_BIN_DIR) ; $(PY) ../../dotfiles/scripts/runalltests.py

# Utility goals for scripts and such.
.PHONY: setup
setup:
	git submodule init
	git submodule update --remote --merge

.PHONY: hooks
hooks: $(SCRIPTS_DIR)/setuphooks.py
	$(PY) $(SCRIPTS_DIR)/setuphooks.py

.PHONY: format
format: $(SCRIPTS_DIR)/format.py
	$(PY) $(SCRIPTS_DIR)/format.py src
################################################################################
