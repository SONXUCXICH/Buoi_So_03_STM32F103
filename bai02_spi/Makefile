 .RECIPEPREFIX := >


TARGET = bai02_spi


CC = arm-none-eabi-gcc

OBJCOPY = arm-none-eabi-objcopy

SIZE = arm-none-eabi-size


CPU = -mcpu=cortex-m3 -mthumb


CFLAGS = $(CPU) -O0 -g -Wall

CFLAGS += -ffunction-sections -fdata-sections

CFLAGS += -DSTM32F10X_MD

CFLAGS += -DUSE_STDPERIPH_DRIVER


CFLAGS += -I.

CFLAGS += -ICMSIS/CM3/CoreSupport

CFLAGS += -ICMSIS/CM3/DeviceSupport/ST/STM32F10x

CFLAGS += -ISTM32F10x_StdPeriph_Driver/inc


SRCDIR = STM32F10x_StdPeriph_Driver/src


OBJS = main.o

OBJS += system_stm32f10x.o

OBJS += gpio.o

OBJS += rcc.o

OBJS += spi.o

OBJS += startup.o


.PHONY: all clean flash


all: $(TARGET).bin


main.o: main.c stm32f10x_conf.h

>$(CC) $(CFLAGS) -c main.c -o main.o


system_stm32f10x.o: system_stm32f10x.c

>$(CC) $(CFLAGS) -c $< -o $@


gpio.o: $(SRCDIR)/stm32f10x_gpio.c

>$(CC) $(CFLAGS) -c $< -o $@


rcc.o: $(SRCDIR)/stm32f10x_rcc.c

>$(CC) $(CFLAGS) -c $< -o $@


spi.o: $(SRCDIR)/stm32f10x_spi.c

>$(CC) $(CFLAGS) -c $< -o $@


startup.o: startup.s

>$(CC) $(CPU) -c $< -o $@


$(TARGET).elf: $(OBJS) linker.ld

>$(CC) $(CPU) -nostdlib -Tlinker.ld -Wl,--gc-sections -Wl,-Map=$(TARGET).map $(OBJS) -lgcc -o $@


$(TARGET).bin: $(TARGET).elf

>$(OBJCOPY) -O binary $< $@

>$(SIZE) $<


clean:

>rm -f *.o *.elf *.bin *.hex *.map


flash: $(TARGET).bin

>st-flash write $(TARGET).bin 0x08000000