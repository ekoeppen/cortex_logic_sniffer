# Declare paths
LIB_PATH = ../..
EM_CMSIS_3 = $(LIB_PATH)/EM_CMSIS_3
DRIVER =  $(EM_CMSIS_3)/emlib
CMSIS_CORE = $(EM_CMSIS_3)/CMSIS/Include
CMSIS_DEVICE = $(EM_CMSIS_3)/Device/EnergyMicro/EFM32G

# OpenOCD interface file used for programming/debugging the micronctroller
OPENOCD_INTERFACE ?= interface/olimex-arm-usb-tiny-h.cfg

# Declare command line tools - assume these are in the path
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
CP = arm-none-eabi-objcopy
OD = arm-none-eabi-objdump

# Declare command line flags
CORE_CFLAGS = -I./ -I$(CMSIS_CORE) -I$(CMSIS_DEVICE)/Include -I$(DRIVER)/inc -fno-common -O0 -g -Wall -mcpu=cortex-m3 -mthumb -DEFM32G210F128
CFLAGS = $(CORE_CFLAGS) -c 
CFLAGS_LINK = -Wl,-T$(CMSIS_DEVICE)/Source/GCC/efm32g.ld -nostartfiles $(CORE_CFLAGS)
ASFLAGS = -mcpu=cortex-m3 -mthumb -g
LDFLAGS = -T$(CMSIS_DEVICE)/Source/GCC/efm32g.ld -nostartfiles
CPFLAGS = -Obinary
ODFLAGS	= -S

# List common and system library source files
SRCS = $(CMSIS_DEVICE)/Source/system_efm32g.c
SRCS += $(CMSIS_DEVICE)/Source/GCC/startup_efm32g.S
SRCS += $(DRIVER)/src/em_cmu.c
SRCS += $(DRIVER)/src/em_gpio.c
SRCS += $(DRIVER)/src/em_emu.c
SRCS += $(DRIVER)/src/em_timer.c
SRCS += $(DRIVER)/src/em_system.c
SRCS += $(DRIVER)/src/em_usart.c
SRCS += em_32g210f128_h.c
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
