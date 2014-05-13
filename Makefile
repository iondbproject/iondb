################################################################################
### 	Makefile for database.
###	Author: Graeme Douglas
###	
###	Copyright 2013 Graeme Douglas
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

# Directory structure.
SRC       := src
BIN       := bin
BIN_LIB   := $(BIN)/lib
BIN_TESTS := $(BIN)/tests
BIN_UTILS := $(BIN)/utils
DOC       := doc

# Compiler options
GCC           =  gcc
CC            =  $(GCC)
CFLAGS        := $(CFLAGS) -Wall -g
OUTPUT_OPTION =  -o $@
################################################################################

## Functions ###################################################################
#(call transform-csource,source-file,new-prefix,new-ending)
define transform-csource
 $(addprefix $2,$(subst .c,$3,$(notdir $1)))
endef

# Generate a single library compilation rule.
#(call gen-lib-rule,source-file)
define gen-lib-rule
 $(call transform-csource,$1,$(BIN_LIB)/,.o): $1 $(subst .c,.h,$1)
	$$(call make-depend,$$<, $$@, $$(subst .o,.d,$$@))
	$(COMPILE.c) $$< $(CFLAGS) -o $$@
endef

# Generate a single library compilation rule.
#(call gen-testlib-rule,source-file)
define gen-testlib-rule
 $(call transform-csource,$1,$(BIN_TESTS)/,.o): $1
	$$(call make-depend,$$<, $$@, $$(subst .o,.d,$$@))
	$(COMPILE.c) $$< $(CFLAGS) -o $$@
endef

# $(call transform-csource,$(subst run_,,$1),$(BIN_TEST)/,): $1
# Generate a single library compilation rule.
#(call gen-test-rule,source-file)
define gen-test-rule
 $(call transform-csource,$1,$(BIN_TESTS)/,): $1
	$$(call make-depend,$$<, $$@, $$(addsuffix .d,$$@))
	$(CC) $(CFLAGS) -o $$@ $$< $(libs) $(testlibs)
endef

# Generate a single library compilation rule.
#(call gen-test-rule,source-file)
define gen-util-rule
 $(call transform-csource,$1,$(BIN_UTILS)/,): $1
	$$(call make-depend,$$<, $$@, $$(addsuffix .d,$$@))
	$(CC) $(CFLAGS) -o $$@ $$< $(libs)
endef

# If this doesn't work, an ugly SED-based solution is required.
#(call make-depend,source-file,object-file,depend-file)
define make-depend
$(GCC)	-MM                     \
        -MF $3                  \
        -MP                     \
        -MT $2                  \
        $(CFLAGS)               \
        $(CPPFLAGS)             \
        $(TARGET_ARCH)          \
        $1
endef
################################################################################

## Sources #####################################################################
# There, for now, are four broad categories for sources.  There are libs,
# which are for the object files of the database, utils, executables used for
# various activities such as test relation generation, test libs, which
# define all of the tests to be run, and test executables, which execute
# the tests.  Simply add the path to the source under the correct category,
# and the object file or executable will automatically be compiled
# and checked for dependencies.

# Sources for database library.
libsources :=  $(SRC)/storage/storage.c \
               $(SRC)/storage/memory/memorystorage.c \

# Generate list of libraries to compile.
libs        := $(addprefix $(BIN_LIB)/,$(subst .c,.o,$(notdir $(libsources))))

# Generate list of dependency files for each file.
libdepends  := $(addprefix $(BIN_LIB)/,$(subst .c,.d,$(notdir $(libsources))))

# TODO: Either remove this or buil utilities. :)
# List of test executable sources.
utilsources :=

# Generate list of utilities to compile.
utilexecs   := $(addprefix $(BIN_UTILS)/,$(subst .c,,$(notdir $(utilsources))))

# Generate list of utility dependencies files.
utildepends := $(addprefix $(BIN_UTILS)/,$(subst .c,.d,$(notdir $(utilssources))))

# List of test library sources.
tlsources   := $(SRC)/tests/CuTest.c \
               $(SRC)/tests/unit/storage/memory/memorystorage.c

# Generate list of libraries to compile.
testlibs    := $(addprefix $(BIN_TESTS)/,$(subst .c,.o,$(notdir $(tlsources))))

# Generate list of dependency files for each file.
tldepends   := $(addprefix $(BIN_TESTS)/,$(subst .c,.d,$(notdir $(tlsources))))

# List of executable test library sources.
testsources := $(SRC)/tests/unit/storage/memory/run_memorystorage.c

# Generate list of libraries to compile.
testexecs   := $(addprefix $(BIN_TESTS)/,$(subst .c,,$(notdir $(testsources))))

# Generate list of dependency files for each file.
testdepends :=$(addprefix $(BIN_TESTS)/,$(subst .c,.d,$(notdir $(testsources))))
################################################################################

## Targets #####################################################################
.PHONY: all
all: init_dirs $(libs) $(utilexecs)
	@echo "Build complete!"

.PHONY: tests
tests: init_dirs $(libs) $(testlibs) $(testexecs) $(utilexecs)
	@echo "Build complete!"

.PHONY: utils
utils: init_dirs $(libs) $(utilexecs)
	@echo "Build complete!"

.PHONY: fresh
fresh:
	make clean
	make tests
	@echo "Build complete!"

.PHONY: clean
clean: init_dirs
	$(RM) $(BIN_LIB)/*
	$(RM) $(BIN_TESTS)/*
	$(RM) $(BIN_UTILS)/*

.PHONY: docs
docs:
	cd $(DOC)/doxygen ; doxygen Doxyfile
	@echo "Generated API reference in $(DOC)/doxygen/{html, latex}"

.PHONY: init_dirs
init_dirs:
	$(MKDIR) $(BIN_LIB)
	$(MKDIR) $(BIN_TESTS)
	$(MKDIR) $(BIN_UTILS)

# Build up object dependencies.
$(testlibs): $(libs)
$(testexecs): $(testlibs)
$(utilexecs): $(libs)

# Generate the list of library rules.
$(foreach source,$(libsources),$(eval $(call gen-lib-rule,$(source))))

# Generate the list of library rules.
$(foreach source,$(utilsources),$(eval $(call gen-util-rule,$(source))))

# Generate a list of test rules.
$(foreach source,$(tlsources),$(eval $(call gen-testlib-rule,$(source))))

# Generate a list of test rules.
$(foreach source,$(testsources),$(eval $(call gen-test-rule,$(source))))
#$(foreach source,$(testsources),$(info $(call gen-test-rule,$(source))))

ifneq "$(MAKECMDGOALS)" "clean"
 -include $(libdepends)
endif

ifeq "$(MAKECMDGOALS)" "tests"
 -include $(tldepends)
 -include $(testdepends)
 -include $(utildepends)
endif

ifeq "$(MAKECMDGOALS)" "utils"
 -include $(utildepends)
endif
################################################################################
