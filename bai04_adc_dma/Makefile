TARGET = bai04_adc_dma

CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CPU = -mcpu=cortex-m3 -mthumb

CFLAGS = $(CPU) \
    -Wall \
    -O0 \
    -ffunction-sections \
    -fdata-sections \
    -DSTM32F10X_MD \
    -DUSE_STDPERIPH_DRIVER

CFLAGS += \
    -I. \
    -ICMSIS/CM3/CoreSupport \
    -ICMSIS/CM3/DeviceSupport/ST/STM32F10x \
    -ISTM32F10x_StdPeriph_Driver/inc

LDFLAGS = $(CPU) \
    -Tlinker.ld \
    -Wl,--gc-sections \
    -specs=nosys.specs

C_SOURCES = \
    main.c \
    system_stm32f10x.c \
    STM32F10x_StdPeriph_Driver/src/misc.c \
    STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c \
    STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c \
    STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c \
    STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c \
    STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c \
    STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c

ASM_SOURCES = startup.s

C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.s=.o)

OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

all: $(TARGET).elf $(TARGET).bin

$(TARGET).elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(CPU) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).bin

flash: $(TARGET).bin
	st-flash --reset write $(TARGET).bin 0x08000000