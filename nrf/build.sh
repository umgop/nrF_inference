#!/bin/bash

# Build script for nRF52840 DK
# Compile, link, and generate Intel HEX file

set -e

# Compiler settings for nRF52840 (Cortex-M4 with FPU)
CC=arm-none-eabi-gcc
OBJCOPY=arm-none-eabi-objcopy
CFLAGS="-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb"
CFLAGS="$CFLAGS -fno-builtin -fno-strict-aliasing -Wall -Wextra -O2"
LDFLAGS="-nostartfiles -Tlinker.ld"

echo "Building for nRF52840 DK..."
echo "Step 1: Compiling C files..."

# Compile startup.c
$CC $CFLAGS -c startup.c -o startup.o
echo "  ✓ startup.o compiled"

# Compile main.c
$CC $CFLAGS -c main.c -o main.o
echo "  ✓ main.o compiled"

echo "Step 2: Linking..."

# Link ELF
$CC $CFLAGS $LDFLAGS startup.o main.o -o main.elf
echo "  ✓ main.elf linked"

echo "Step 3: Generating HEX..."

# Generate Intel HEX
$OBJCOPY -O ihex main.elf main.hex
echo "  ✓ main.hex generated"

# Verify
echo ""
echo "Build artifacts:"
ls -lh main.elf main.hex
echo ""
echo "✓ Build complete! Ready to flash."
echo ""
echo "To flash to nRF52840 DK:"
echo "  nrfjprog --ids                                                    # List connected devices"
echo "  nrfjprog --recover -f NRF52840                                    # Erase (one-time)"
echo "  nrfjprog --program main.hex --verify --sectorerase -f NRF52840    # Program"
echo "  nrfjprog --reset -f NRF52840                                      # Reset and run"
