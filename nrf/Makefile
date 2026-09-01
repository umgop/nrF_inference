# Makefile for nRF52840 DK Embedded Project

# Tools
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size

# Target
TARGET = main

# Compiler flags for nRF52840 (Cortex-M4 with FPU)
CPU = cortex-m4
FPU = fpv4-sp-d16
FLOAT_ABI = hard

CFLAGS = -mcpu=$(CPU)
CFLAGS += -mfpu=$(FPU)
CFLAGS += -mfloat-abi=$(FLOAT_ABI)
CFLAGS += -mthumb
CFLAGS += -Wall -Wextra -O2
CFLAGS += -fno-builtin -fno-strict-aliasing
CFLAGS += -ffreestanding

# Linker flags
LDFLAGS = -nostartfiles -nolibc
LDFLAGS += -Tlinker.ld
LDFLAGS += -Wl,-Map=$(TARGET).map
LDFLAGS += -Wl,--gc-sections

# Source files
SOURCES = startup.c main.c
OBJECTS = $(SOURCES:.c=.o)

# Build targets
.PHONY: all clean program recover reset program-verify

all: $(TARGET).hex $(TARGET).elf
	@echo ""
	@echo "Build complete!"
	@echo "Artifacts:"
	@ls -lh $(TARGET).elf $(TARGET).hex
	@$(SIZE) $(TARGET).elf

$(TARGET).elf: $(OBJECTS)
	@echo "[LINK] $@"
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.c
	@echo "[CC] $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).hex: $(TARGET).elf
	@echo "[HEX] $@"
	$(OBJCOPY) -O ihex $< $@

# Flashing targets
recover:
	nrfjprog --recover -f NRF52

program: $(TARGET).hex
	nrfjprog --program $(TARGET).hex --verify --sectorerase -f NRF52

program-verify: $(TARGET).hex
	nrfjprog --program $(TARGET).hex --verify --sectorerase -f NRF52
	nrfjprog --verify $(TARGET).hex -f NRF52

reset:
	nrfjprog --reset -f NRF52

ids:
	nrfjprog --ids

full-flash: recover program reset
	@echo "✓ Full flash sequence complete!"

clean:
	rm -f $(OBJECTS) $(TARGET).elf $(TARGET).hex $(TARGET).map
	@echo "Clean complete"
