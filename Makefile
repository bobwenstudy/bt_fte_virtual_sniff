#
# 'make'        build executable file 'main'
# 'make clean'  removes all .o and executable files
#
.DEFAULT_GOAL := all

# CROSS_COMPILE 	?= arm-linux-gnueabihf-
CROSS_COMPILE 	?= 
TARGET		  	?= main

# define the C compiler to use
CC 				:= $(CROSS_COMPILE)gcc
LD				:= $(CROSS_COMPILE)ld
OBJCOPY 		:= $(CROSS_COMPILE)objcopy
OBJDUMP 		:= $(CROSS_COMPILE)objdump
SIZE			:= $(CROSS_COMPILE)size


# define any compile-time flags
CFLAGS  ?= -O0
CFLAGS  += -g

# warning param setting
CFLAGS	+= -Wall
# think use -Os.
CFLAGS	+= -Wno-unused-function
CFLAGS	+= -Wno-unused-variable

CFLAGS	+= -Wstrict-prototypes
CFLAGS	+= -Wshadow
# CFLAGS	+= -Werror

# spec c version
CFLAGS  += -std=c99

# for makefile depend tree create
CFLAGS  += -MMD -MP

# for weak.
CFLAGS  += -fno-common

## MAKEFILE COMPILE MESSAGE CONTROL ##
ifeq ($(V),1)
	Q=
else
	Q=@
endif

# Put functions and data in their own binary sections so that ld can
# garbage collect them
ifeq ($(NOGC),1)
GC_CFLAGS =
GC_LDFLAGS =
else
GC_CFLAGS = -ffunction-sections -fdata-sections
GC_LDFLAGS = -Wl,--gc-sections -Wl,--check-sections
endif


CFLAGS	+= $(GC_CFLAGS)

# define ld params
LDFLAGS	:=
LDFLAGS	+= $(GC_LDFLAGS)


# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
LFLAGS := 

# define output directory
OUTPUT_PATH	:= output

# define source directory
SRC		:= 

# define include directory
INCLUDE	:=
INCLUDE	+= output

# define lib directory
LIB		:= 


OUTPUT_TARGET	:= $(OUTPUT_PATH)/$(TARGET)

OBJDIR = $(OUTPUT_PATH)/obj

#set report setting. becareful, this only support elf. not work on windows.
DEBUG_REPORT :=


# include user build.mk
include build.mk




ifeq ($(OS),Windows_NT)
ifdef ComSpec
	WINCMD:=$(ComSpec)
endif
ifdef COMSPEC
	WINCMD:=$(COMSPEC)
endif

SHELL:=$(WINCMD)

MAIN	:= $(TARGET).exe
ECHO=echo
SOURCEDIRS	:= $(SRC)
INCLUDEDIRS	:= $(INCLUDE)
LIBDIRS		:= $(LIB)
FIXPATH = $(subst /,\,$1)
RM			:= del /q /s
MD			:= mkdir
else
MAIN	:= $(TARGET)
ECHO=echo
SOURCEDIRS	:= $(shell find $(SRC) -type d)
INCLUDEDIRS	:= $(shell find $(INCLUDE) -type d)
LIBDIRS		:= $(shell find $(LIB) -type d)
FIXPATH = $1
RM = rm -rf
MD	:= mkdir -p
endif

# define any directories containing header files other than /usr/include
INCLUDES	:= $(patsubst %,-I%, $(INCLUDEDIRS:%/=%))
@echo INCLUDES: $(INCLUDES)

# define the C libs
LIBS		:= $(patsubst %,-L%, $(LIBDIRS:%/=%))

# define the C source files
SOURCES		:= $(wildcard $(patsubst %,%/*.c, $(SOURCEDIRS)))

# define the C object files 
OBJECTS			:= $(patsubst %, $(OBJDIR)/%, $(SOURCES:.c=.o))
OBJ_MD			:= $(addprefix $(OBJDIR)/, $(SOURCEDIRS))



ALL_DEPS := $(OBJECTS:.o=.d)

# include dependency files of application
ifneq ($(MAKECMDGOALS),clean)
-include $(ALL_DEPS)
endif

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

OUTPUT_MAIN		:= $(OUTPUT_PATH)/$(MAIN)

# Fix path error.
#OUTPUT_MAIN := $(call FIXPATH,$(OUTPUT_MAIN))

.PHONY: clean help info

info:
	@$(ECHO) Current Configuration: APP=$(APP) PORT=$(PORT) CHIPSET=$(CHIPSET)

all: | info main $(DEBUG_REPORT)
	@$(ECHO) Start Build Image.
	$(OBJCOPY) -v -O binary $(OUTPUT_MAIN) $(OUTPUT_TARGET).bin
	$(OBJDUMP) --source --all-headers --demangle --line-numbers --wide $(OUTPUT_MAIN) > $(OUTPUT_TARGET).lst
	@$(ECHO) Print Size
	$(Q)@$(SIZE) --format=berkeley $(OUTPUT_MAIN)
#	@$(ECHO) Executing 'all' complete!

# mk path for object.
$(OBJ_MD):
	$(Q)if not exist "$@" $(Q)$(MD) $(call FIXPATH, $@)

# mk output path.
$(OUTPUT_PATH):
	$(Q)if not exist "$@" $(Q)$(MD) $(call FIXPATH, $@)

$(OBJDIR):
	$(Q)if not exist "$@" $(Q)$(MD) $(call FIXPATH, $@)

$(OUTPUT_MAIN): $(OBJECTS)
	@$(ECHO) Linking    : "$@"
	$(Q)$(CC) $(CFLAGS) $(LDFLAGS) $(INCLUDES) -Wl,-Map,$(OUTPUT_TARGET).map -o $(OUTPUT_MAIN) $(OBJECTS) $(LFLAGS) $(LIBS)

main: | $(OUTPUT_PATH) $(OBJDIR) $(OBJ_MD) $(OUTPUT_MAIN)
	@$(ECHO) Building   : "$(OUTPUT_MAIN)"

# Static Pattern Rules [targets ...: target-pattern: prereq-patterns ...]
$(OBJECTS): $(OBJDIR)/%.o : %.c $(AUTOCONFIG_H)
	@$(ECHO) Compiling  : "$<"
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@


clean:
#	$(RM) $(OUTPUT_MAIN)
#	$(RM) $(OBJECTS)
#	$(RM) $(OBJDIR)
	$(Q)$(RM) $(call FIXPATH, $(OUTPUT_PATH))
	@$(ECHO) Cleanup complete!

run: all
	./$(OUTPUT_MAIN)
	@$(ECHO) Executing 'run: all' complete!

ram_report:
	python scripts/footprint/size_report -k $(OUTPUT_MAIN) -z $(OUTPUT_PATH) -o $(OUTPUT_PATH) --workspace=$(OUTPUT_PATH) -d 99 ram

rom_report:
	python scripts/footprint/size_report -k $(OUTPUT_MAIN) -z $(OUTPUT_PATH) -o $(OUTPUT_PATH) --workspace=$(OUTPUT_PATH) -d 99 rom

all_report:
	python scripts/footprint/size_report -k $(OUTPUT_MAIN) -z $(OUTPUT_PATH) -o $(OUTPUT_PATH) --workspace=$(OUTPUT_PATH) -d 99 all

code_format:
	python code_format.py


help:
	@$(ECHO) "EasyBt Software Development Kit"
	@$(ECHO) "== For detailed user guide, please check xxxx"
	@$(ECHO) "== Make variables used in SDK =="
	@$(ECHO) "APP:         Select APP Demo built in SDK, will select <beacon> by default"
	@$(ECHO) "PORT:        Select Porting info built in SDK, will select <windows_libusb_win32> by default"
	@$(ECHO) "CHIPSET:     Select Chipset built in SDK, will select <csr8510> by default"
	@$(ECHO) "NOGC:        NOGC=1 diable gc sections, default is 0"
	@$(ECHO) "V:           V=1 verbose make, will print more information, by default V=0"
	@$(ECHO) "== How to Use with Make =="
	@$(ECHO) "1. Build Application:"
	@$(ECHO) "all [APP=beacon] [PORT=windows_libusb_win32] [CHIPSET=csr8510]"
	@$(ECHO) "   Build a software program."
	@$(ECHO) "2. Show menuconfig:"
	@$(ECHO) "menuconfig [APP=beacon] [PORT=windows_libusb_win32] [CHIPSET=csr8510]"
	@$(ECHO) "   Use menuconfig to show the Kconfig setting."
	@$(ECHO) "3. Show guiconfig:"
	@$(ECHO) "guiconfig [APP=beacon] [PORT=windows_libusb_win32] [CHIPSET=csr8510]"
	@$(ECHO) "   Use guiconfig to show the Kconfig setting."
	@$(ECHO) "4: Format all code style by .clang-format"
	@$(ECHO) "code_format"
	@$(ECHO) "   It is recommended to modify the code format before submitting."
	@$(ECHO) ""
	@$(ECHO) "== Example Usage =="
	@$(ECHO) "1. Build for default application: make all"
	@$(ECHO) "2. Run app(Windows), output\main.exe"
	@$(ECHO) ""