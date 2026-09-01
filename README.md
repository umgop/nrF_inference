# nRF52840 Neural Network Inference Engine

A bare-metal embedded neural network inference implementation running on the **nRF52840 DK** microcontroller. This project demonstrates real-time model execution on an ARM Cortex-M4 with FPU, utilizing hardware sensors and GPIO for I/O.

## Overview

This project trains a small neural network in Jupyter Notebook, extracts the weights, and deploys it directly on an nRF52840 microcontroller for real-time inference. The model reads temperature from the chip's built-in sensor and button state, then outputs a decision via LED pulse.

## Hardware

- **Board:** nRF52840 DK (Nordic nRF52840 microcontroller)
- **Processor:** ARM Cortex-M4 with FPU (64 MHz)
- **Memory:** 256 KB Flash, 64 KB RAM
- **Sensors:** On-chip TEMP sensor
- **I/O:** Button 1 (P0.11), LED 1 (P0.13)

## Model Architecture

A simple 2-layer neural network:

```
Input (2 features)
    ↓
Hidden Layer (2 neurons, ReLU)
    ↓
Output (1 neuron, linear)
```

### Layers

| Layer | Input Dim | Output Dim | Activation |
|-------|-----------|-----------|------------|
| Input | 2 | - | - |
| Hidden | 2 | 2 | ReLU |
| Output | 2 | 1 | Linear |

### Pre-trained Weights

**W1 (Hidden weights):**
```
[[ 0.1423, -0.3102],
 [ 0.8521,  0.4190]]
```

**B1 (Hidden bias):** `[-3.1204, 0.0821]`

**W2 (Output weights):**
```
[[ 1.1042],
 [-0.4891]]
```

**B2 (Output bias):** `[0.0210]`

## Inputs & Outputs

### Inputs

1. **Temperature (°C)**
   - Source: nRF52840 built-in TEMP sensor
   - Range: Typically -40°C to +125°C
   - Read every ~2 seconds

2. **Button State**
   - Source: Button 1 (P0.11), active-low with pull-up
   - Values: `0.0` (not pressed) or `2.0` (pressed)

### Output

- **LED 1 (P0.13):** Pulses ON for ~78ms when model score > 0.0, then OFF
- **Inference Cycle:** Repeats every ~2 seconds

## Project Structure

```
nrf/
├── main.c              # Inference engine + GPIO/TEMP peripheral access
├── startup.c           # Cortex-M4 vector table + FPU initialization
├── linker.ld           # Memory layout (256KB Flash, 64KB RAM)
├── weights.h           # Pre-trained model weights
├── Makefile            # Build configuration
└── README.md           # This file
```

## Building

### Requirements

- ARM GCC Toolchain: `arm-none-eabi-gcc`
- nrfjprog (Nordic nRF command-line programmer)
- make

### Build Steps

```bash
cd /path/to/nrf
make clean
make
```

This generates:
- `main.elf` - Executable ELF file
- `main.hex` - Intel HEX format (for flashing)
- `main.map` - Memory map

## Flashing

### One-Time Setup (Erase Device)

```bash
nrfjprog --recover -f NRF52
```

### Program the Firmware

```bash
nrfjprog --program main.hex --verify --sectorerase -f NRF52
```

### Reset and Run

```bash
nrfjprog --reset -f NRF52
```

### Quick Flash

```bash
make full-flash
```

## How It Works

1. **Initialization:**
   - FPU enabled (Cortex-M4 feature)
   - GPIO pins configured (LED 1, Button 1)
   - Startup LED test blink (10x for verification)

2. **Main Loop:**
   - Reads temperature from TEMP sensor
   - Reads button state (P0.11, active-low)
   - Runs inference: `output = W2 · ReLU(W1 · [temp, button] + B1) + B2`
   - If `output > 0.0`: Pulse LED 1 ON for ~78ms
   - Sleep ~2 seconds, repeat

3. **Inference Calculation:**
   ```c
   float score = run_inference(temp_c, button_hold_sec);
   ```
   - Forward pass through 2 hidden neurons with ReLU
   - Linear output layer
   - Single scalar output drives LED decision

## Memory Layout

**Flash (256 KB):**
- `.isr_vector` - Exception vectors
- `.text` - Code
- `.rodata` - Read-only data (weights)
- `.data` - Initialized data (loaded from flash)

**RAM (64 KB):**
- `.data` - Initialized data (copied from flash)
- `.bss` - Zero-initialized data
- Runtime stack

## Peripheral Access

### Temperature Sensor (TEMP)

- Start: Write 1 to `0x4000C000` (TASKS_START)
- Wait: Poll `0x4000C100` (EVENTS_DATARDY)
- Read: `0x4000C508` (TEMP register, divide by 4 for °C)
- Stop: Write 1 to `0x4000C004` (TASKS_STOP)

### GPIO (P0)

- Input: `0x50000510` (P0_IN register)
- Output Set: `0x50000508` (P0_OUTSET, write 1 to set high)
- Output Clear: `0x5000050C` (P0_OUTCLR, write 1 to set low)
- Direction: `0x50000718` (P0_DIRSET, write 1 to output)
- Config: `0x50000700 + 4*n` (P0_PIN_CNF[n] for pin n)

## Compiler Flags

```
-mcpu=cortex-m4           # ARM Cortex-M4 target
-mfpu=fpv4-sp-d16         # Single-precision FPU
-mfloat-abi=hard          # Hardware floating-point ABI
-mthumb                   # Thumb instruction set
-ffreestanding            # Freestanding C environment
-nostartfiles             # No startup files
-nolibc                   # No C standard library
```

## Performance

- **Model Size:** ~500 bytes (code + weights)
- **Inference Time:** <1ms (64 MHz Cortex-M4)
- **Cycle Time:** ~2 seconds (dominated by sensor read + delay)
- **Power:** Ultra-low (embedded nRF52840 power management)

## Model Training

The weights were trained in Jupyter Notebook on a dataset with:
- **Features:** Temperature (°C), Button hold duration (seconds)
- **Labels:** Binary classification (LED on/off decision)
- **Training:** Simple 2-layer network with ReLU hidden layer

Weights extracted and hardcoded into `weights.h`.

## References

- [nRF52840 Product Specification](https://infocenter.nordicsemi.com/pdf/nRF52840_PS_v3.3.pdf)
- [ARM Cortex-M4 Generic User Guide](https://developer.arm.com/documentation/100166/latest/)
- [nRFx HAL Documentation](https://github.com/NordicPlayground/nrfx)

## License

MIT License - See LICENSE file for details

## Author

Umesh Gopi - September 2026
