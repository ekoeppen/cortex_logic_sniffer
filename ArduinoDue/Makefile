# Declare paths
LIB_PATH = ../..
SAM_PATH = $(LIB_PATH)/asf-3.5.1
DRIVER =  $(SAM_PATH)/sam
CMSIS_CORE = $(SAM_PATH)/thirdparty/CMSIS

# OpenOCD interface file used for programming/debugging the micronctroller
OPENOCD_INTERFACE ?= interface/olimex-arm-usb-tiny-h.cfg

# Declare command line tools - assume these are in the path
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
CP = arm-none-eabi-objcopy
OD = arm-none-eabi-objdump

# Declare command line flags
CORE_CFLAGS = -I. -I../common -I$(CMSIS_CORE)/Include -I$(SAM_PATH)/common/utils -I$(DRIVER)/drivers -I$(DRIVER)/utils -I$(DRIVER)/utils/preprocessor -I$(DRIVER)/utils/header_files -I$(DRIVER)/utils/cmsis/sam3x/include -I$(DRIVER)/utils/cmsis/sam3x/source/templates -fno-common -O0 -g -Wall -mcpu=cortex-m3 -mthumb -D__SAM3X8E__
CFLAGS = $(CORE_CFLAGS) -c 
CFLAGS_LINK = -Wl,-T$(DRIVER)/utils/linker_scripts/sam3x/sam3x8/gcc/flash.ld -nostartfiles -nodefaultlibs $(CORE_CFLAGS)
ASFLAGS = -mcpu=cortex-m3 -mthumb -g
LDFLAGS = -T$(DRIVER)/utils/linker_scripts/sam3x/sam3x8/gcc/flash.ld -nostartfiles
CPFLAGS = -Obinary
ODFLAGS	= -S

# List common and system library source files
# (i.e. for accessing STM32/Cortex M3 hardware) 
SRCS = $(DRIVER)/utils/cmsis/sam3x/source/templates/system_sam3x.c
SRCS += $(DRIVER)/utils/cmsis/sam3x/source/templates/exceptions.c
SRCS += $(DRIVER)/utils/cmsis/sam3x/source/templates/gcc/startup_sam3x.c
SRCS += $(DRIVER)/drivers/pmc/pmc.c
SRCS += $(DRIVER)/drivers/pio/pio.c
SRCS += $(DRIVER)/drivers/uart/uart.c
SRCS += arduino_due.c
SRCS += ../common/main.c ../common/sampler.s

# List path to demo build output files
OUTPUT_FILES = $(addsuffix /main,$(DEMO_FOLDERS))
ELF_FILES = $(addsuffix .elf,$(OUTPUT_FILES))
LIST_FILES = $(addsuffix .list,$(OUTPUT_FILES))
BIN_FILE = $(addsuffix .bin,$(OUTPUT_FILES))

# Generic targets
.PHONY: clean all prog dbg

all: main.elf main.bin main.lst 

clean:
	find . -type f -name "*.o" -exec rm {} \;
	find . -type f -name "*.elf" -exec rm {} \;
	find . -type f -name "*.bin" -exec rm {} \;
	find . -type f -name "*.lst" -exec rm {} \; 

# Targets to program a microntroller using OpenOCD
prog:
	-killall -q openocd
	openocd -f $(OPENOCD_INTERFACE) -f openocd/openocd_stm32_p103.cfg -c "program_flash main.bin"

# Target to launch OpenOCD - by default, OpenOCD creates a GDB server at port 3333.
dbg:
	-killall -q openocd
	openocd -f $(OPENOCD_INTERFACE) -f openocd/openocd_stm32_p103.cfg -c "init_stm32"
	
%.lst : %.elf
	$(OD) $(ODFLAGS) $< > $@

%.bin : %.elf
	$(CP) $(CPFLAGS) $< $@

main.elf: $(SRCS)
	$(CC) $(CFLAGS_LINK) -o $@ $^
