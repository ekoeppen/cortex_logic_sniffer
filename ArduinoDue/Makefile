# Declare paths
LIB_PATH = ../..
SAM_PATH = $(LIB_PATH)/SAM3
DRIVER =  $(SAM_PATH)/libsam
CMSIS_CORE = $(SAM_PATH)/CMSIS/CMSIS
CMSIS_DEVICE = $(SAM_PATH)/CMSIS/Device/ATMEL

# OpenOCD interface file used for programming/debugging the micronctroller
OPENOCD_INTERFACE ?= interface/olimex-arm-usb-tiny-h.cfg

# Declare command line tools - assume these are in the path
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
CP = arm-none-eabi-objcopy
OD = arm-none-eabi-objdump

# Declare command line flags
CORE_CFLAGS = -I. -I../common -I$(CMSIS_CORE)/Include -I$(CMSIS_DEVICE) -I$(CMSIS_DEVICE)/sam3xa/include -I$(DRIVER)/include -fno-common -O0 -g -Wall -mcpu=cortex-m3 -mthumb -D__SAM3X8E__
CFLAGS = $(CORE_CFLAGS) -c 
CFLAGS_LINK = -Wl,-T$(CMSIS_DEVICE)/sam3xa/source/gcc_arm/sam3x8e_flash.ld -L$(CMSIS_DEVICE)/sam3xa/source/gcc_arm -nostartfiles -nodefaultlibs $(CORE_CFLAGS)
ASFLAGS = -mcpu=cortex-m3 -mthumb -g
LDFLAGS = -T$(CMSIS_DEVICE)/sam3xa/source/gcc_arm/sam3x8e_flash.ld -nostartfiles
CPFLAGS = -Obinary
ODFLAGS	= -S

# List common and system library source files
# (i.e. for accessing STM32/Cortex M3 hardware) 
SRCS = $(CMSIS_DEVICE)/sam3xa/source/system_sam3xa.c
SRCS += $(CMSIS_DEVICE)/sam3xa/source/gcc_arm/startup_sam3xa.c
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