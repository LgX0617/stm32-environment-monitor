TARGET := EM_System
BUILD := build
CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS := -mcpu=cortex-m3 -mthumb -mfloat-abi=soft -std=gnu11 -Wall -Wextra -ffunction-sections -fdata-sections -g3 -O0 -DUSE_HAL_DRIVER -DSTM32F103xE
CPPFLAGS := -ICore/Inc -IBSP/Inc -IApp/Inc -IDrivers/STM32F1xx_HAL_Driver/Inc -IDrivers/STM32F1xx_HAL_Driver/Inc/Legacy -IDrivers/CMSIS/Device/ST/STM32F1xx/Include -IDrivers/CMSIS/Include
LDFLAGS := -TSTM32F103ZE.ld -mcpu=cortex-m3 -mthumb -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map --specs=nano.specs --specs=nosys.specs
SOURCES := $(wildcard Core/Src/*.c) $(wildcard BSP/Src/*.c) $(wildcard App/Src/*.c) $(wildcard Drivers/STM32F1xx_HAL_Driver/Src/*.c) startup_stm32f103xe_gcc.s
OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(filter %.c,$(SOURCES))) $(BUILD)/startup_stm32f103xe_gcc.o

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).hex $(BUILD)/$(TARGET).bin
$(BUILD)/%.o: %.c
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@
$(BUILD)/startup_stm32f103xe_gcc.o: startup_stm32f103xe_gcc.s
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(CFLAGS) -c $< -o $@
$(BUILD)/$(TARGET).elf: $(OBJECTS)
	@powershell -NoProfile -Command "New-Item -ItemType Directory -Force -Path '$(dir $@)' | Out-Null"
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex $< $@
$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@
clean:
	powershell -NoProfile -Command "if (Test-Path '$(BUILD)') { Remove-Item -Recurse -Force '$(BUILD)' }"
-include $(OBJECTS:.o=.d)
.PHONY: all clean
