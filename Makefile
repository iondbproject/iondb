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
AVR 	  := "C:\Program Files (x86)\Arduino\hardware\tools\avr\avr\include\avr"
BIN       := bin
BIN_LIB   := $(BIN)/lib
BIN_AVR   := $(BIN)/avr
BIN_TESTS := $(BIN)/tests
BIN_UTILS := $(BIN)/utils
BIN_TARGET:= $(BIN)/target
DOC       := doc
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

# If this doesn't work, an ugly SED-based solution is required.
#(call make-depend,source-file,object-file,depend-file)
define make-avr-depend
$(AVR_GCC) -MM                     \
        -MF $3                  \
        -MP                     \
        -MT $2                  \
		-D$(AVR_PROC)			\
		$(AVR_INC)	 			\
        $(AVR_CFLAGS)           \
        $(CPPFLAGS)             \
        $(AVR_TARGET_ARCH)      \
        $1
endef

# Generate a single library compilation rule.
#(call gen-lib-rule,source-file)
define gen-avrlib-rule
 $(call transform-csource,$1,$(BIN_AVR)/,.o): $1 $(subst .c,.h,$1)
	$$(call make-avr-depend,$$<, $$@, $$(subst .o,.d,$$@))
	$(AVR_GCC) -std=c99 -D$(AVR_PROC) $$< $(AVR_CFLAGS) $(AVR_TARGET_ARCH) $(AVR_INC) -c -o $$@
endef

# $(call transform-csource,$(subst run_,,$1),$(BIN_TEST)/,): $1
# Generate a single library compilation rule.
#(call gen-test-rule,source-file)
define gen-avrtarget-rule
 $(call transform-csource,$1,$(BIN_TARGET)/,): $1
	$$(call make-avr-depend,$$<, $$@, $$(addsuffix .d,$$@))
	$(AVR_GCC) -std=c99 -D$(AVR_PROC) $(AVR_TARGET_ARCH) $(AVR_CFLAGS) $(AVR_INC) -c -o $$@.o $$< 
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
libsources :=  	$(SRC)/kv_io.c \
				$(SRC)/dictionary.c \
				$(SRC)/slhandler.c \
				$(SRC)/slstore.c \
				$(SRC)/oadictionaryhandler.c \
				$(SRC)/oahash.c \
				$(SRC)/ffdictionaryhandler.c \
				$(SRC)/flatfile.c \
				$(SRC)/oafdictionaryhandler.c \
				$(SRC)/oafhash.c 
	
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
tlsources   := 	$(SRC)/CuTest.c  \
				$(SRC)/test_slstore.c	\
				$(SRC)/test_slhandler.c \
				$(SRC)/test_dictionary.c \
				$(SRC)/test_oahash.c	\
				$(SRC)/test_oadictionaryhandler.c \
				$(SRC)/test_ffdictionaryhandler.c \
				$(SRC)/test_flatfile.c \
				$(SRC)/test_oafhash.c \
				$(SRC)/test_oafdictionaryhandler.c

# Generate list of libraries to compile.
testlibs    := $(addprefix $(BIN_TESTS)/,$(subst .c,.o,$(notdir $(tlsources))))

# Generate list of dependency files for each file.
tldepends   := $(addprefix $(BIN_TESTS)/,$(subst .c,.d,$(notdir $(tlsources))))

# List of executable test library sources. (main)
testsources := 	$(SRC)/skiplist.c	\
				$(SRC)/hashmap.c 	\
				$(SRC)/run_slstore.c \
				$(SRC)/run_dictionary.c \
				$(SRC)/run_oahash.c \
				$(SRC)/run_flatfile.c \
				$(SRC)/run_oafhash.c

# Generate list of libraries to compile.
testexecs   := $(addprefix $(BIN_TESTS)/,$(subst .c,,$(notdir $(testsources))))

# Generate list of dependency files for each file.
testdepends :=$(addprefix $(BIN_TESTS)/,$(subst .c,.d,$(notdir $(testsources))))

# Sources for database library.
avrlibsrcs :=  	$(SRC)/kv_io.c \
				$(SRC)/serial.c	\
				$(SRC)/dictionary.c \
				$(SRC)/slhandler.c \
				$(SRC)/slstore.c \
				$(SRC)/ramutil.c \
				$(SRC)/benchmark.c
					
# list of target test sources for Atmel Procs
avrtargetsrc := $(SRC)/sample.c \
				$(SRC)/ramcheck.c \
				$(SRC)/benchmark_prototype.c \
				$(SRC)/ard_limits.c \
				$(SRC)/ion_bench.c

# Generate list of libraries to compile.
avrlibs        := $(addprefix $(BIN_AVR)/,$(subst .c,.o,$(notdir $(avrlibsrcs))))

# Generate list of dependency files for each file.
avrlibdepends  := $(addprefix $(BIN_AVR)/,$(subst .c,.d,$(notdir $(avrlibsrcs))))

# Generate list of libraries to compile for avr
avrexecs   := $(addprefix $(BIN_TARGET)/,$(subst .c,,$(notdir $(avrtargetsrc))))
				
################################################################################

## Targets #####################################################################
.PHONY: all
# Compiler options for all
GCC           =  gcc
CC            =  $(GCC)
CFLAGS        := $(CFLAGS) -Wall -g
OUTPUT_OPTION =  -o $@
all: init_dirs $(libs) $(utilexecs)
	@echo "Build complete!"
	
.PHONY: tests
# Compiler options for tests - builds on host
GCC           =  gcc
CC            =  $(GCC)
CFLAGS        := $(CFLAGS) -Wall -g
OUTPUT_OPTION =  -o $@
tests: init_dirs $(libs) $(testlibs) $(testexecs) $(utilexecs)
	@echo "Build complete!"

.PHONY: utils
# Compiler options for utils
GCC           =  gcc
CC            =  $(GCC)
CFLAGS        := $(CFLAGS) -Wall -g
OUTPUT_OPTION =  -o $@
utils: init_dirs $(libs) $(utilexecs)
	@echo "Build complete!"

.PHONY: fresh
fresh:
	make clean
	make tests
	@echo "Build complete!"

#compiler options for mega2560
.PHONY: mega
mega: 
	make FILE=$(FILE) AVR_PROC=atmega2560 avr

.PHONY: uno
# $(avrtargetsrc) $(avrlibsrc) 
# Compiler options for uno
uno: 
	make FILE=$(FILE) AVR_PROC=atmega328p avr

.PHONY: avr
AVR_GCC       =  avr-gcc
AVR_CC        =  $(AVR_GCC)
AVR_CFLAGS    =  -Wall -g -DF_CPU=16000000UL -c
AVR_TARGET_ARCH  =  -mmcu=$(AVR_PROC)
AVR_INC		  = -I'C:\Program Files (x86)\Arduino\hardware\tools\avr\avr\include\'	
OUTPUT_OPTION =  -o $@.o
OBJ_OPTION	  = -o $(BIN_TARGET)/$(subst .c,,$(FILE))
avr: avr_init_dirs $(avrlibs) $(avrexecs)
	$(AVR_CC) $(AVR_TARGET_ARCH) $(BIN_TARGET)/$(subst .c,.o,$(FILE)) $(OBJ_OPTION) $(avrlibs)
	avr-objcopy -O ihex -R .eeprom $(BIN_TARGET)/$(subst .c,,$(FILE)) $(BIN_TARGET)/$(subst .c,.hex,$(FILE))
	@echo "Build complete!"
		
.PHONY: clean_avr
clean_avr:	
	$(RM) avr.o 
	$(RM) avr.hex
	$(RM) $(BIN_AVR)/*
	$(RM) $(BIN_TARGET)/*
	
.PHONY: prog_uno
prog_uno: clean_avr 
	make FILE=$(FILE) uno
	avrdude -F -V -c arduino -p ATMEGA328P -P $(PORT) -b 115200 -U flash:w:$(BIN_TARGET)/$(subst .c,.hex,$(FILE))
	
.PHONY: prog_mega
prog_mega: clean_avr 
	make FILE=$(FILE) mega
	avrdude -F -V -c stk500v2 -p atmega2560 -P $(PORT) -b 115200 -U flash:w:$(BIN_TARGET)/$(subst .c,.hex,$(FILE))

.PHONY: clean
clean: init_dirs
	$(RM) $(BIN_LIB)/*
	$(RM) $(BIN_TESTS)/*
	$(RM) $(BIN_UTILS)/*
	$(RM) $(BIN_TARGET)/*
	
.PHONY: docs
docs:
	cd $(DOC)/doxygen ; doxygen Doxyfile
	@echo "Generated API reference in $(DOC)/doxygen/{html, latex}"

.PHONY: init_dirs
init_dirs:
	$(MKDIR) $(BIN_LIB)
	$(MKDIR) $(BIN_TESTS)
	$(MKDIR) $(BIN_UTILS)
	$(MKDIR) $(BIN_TARGET)
	
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

.PHONY: avr_init_dirs
avr_init_dirs:
	$(MKDIR) $(BIN_AVR)
	$(MKDIR) $(BIN_TARGET)

#build up dependancies
$(avrexecs): $(avrlibs)	

# Generate the list of library rules.
$(foreach source,$(avrlibsrcs),$(eval $(call gen-avrlib-rule,$(source))))

## Generate target
$(foreach source,$(avrtargetsrc),$(eval $(call gen-avrtarget-rule,$(source))))

ifeq "$(MAKECMDGOALS)" "uno"
 -include $(avrlibdepends) 
endif
################################################################################