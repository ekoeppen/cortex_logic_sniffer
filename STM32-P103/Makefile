# Declare paths
LIB_PATH = ../..
STD_PERIPH = $(LIB_PATH)/STM32F10x_StdPeriph_Lib_V3.5.0
DRIVER =  $(STD_PERIPH)/Libraries/STM32F10x_StdPeriph_Driver
CMSIS_CORE = $(STD_PERIPH)/Libraries/CMSIS/CM3/CoreSupport
CMSIS_DEVICE = $(STD_PERIPH)/Libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x

# OpenOCD interface file used for programming/debugging the micronctroller
OPENOCD_INTERFACE ?= interface/olimex-arm-usb-tiny-h.cfg

# Declare command line tools - assume these are in the path
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
CP = arm-none-eabi-objcopy
OD = arm-none-eabi-objdump

# Declare command line flags
CORE_CFLAGS = -I./ -I$(CMSIS_CORE) -I$(CMSIS_DEVICE) -I$(DRIVER)/inc -fno-common -O0 -g -mcpu=cortex-m3 -mthumb -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
CFLAGS = $(CORE_CFLAGS) -c 
CFLAGS_LINK = -Wl,-Tstm32f103.ld -nostartfiles $(CORE_CFLAGS)
ASFLAGS = -mcpu=cortex-m3 -mthumb -g
LDFLAGS = -Tstm32f103.ld -nostartfiles
CPFLAGS = -Obinary
ODFLAGS	= -S

# List common and system library source files
# (i.e. for accessing STM32/Cortex M3 hardware) 
SRCS = $(CMSIS_CORE)/core_cm3.c
SRCS += $(CMSIS_DEVICE)/system_stm32f10x.c
SRCS += $(CMSIS_DEVICE)/startup/gcc_ride7/startup_stm32f10x_md.s
SRCS += $(DRIVER)/src/stm32f10x_rcc.c
SRCS += $(DRIVER)/src/stm32f10x_gpio.c
SRCS += $(DRIVER)/src/stm32f10x_usart.c
SRCS += $(DRIVER)/src/stm32f10x_exti.c
SRCS += $(DRIVER)/src/misc.c
SRCS += stm32_p103.c
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
