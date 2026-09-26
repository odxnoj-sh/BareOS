# Hardware Documentation

## ESP32-S3 Specifications

### CPU

- Xtensa LX7 dual-core
- 240 MHz max frequency
- 32-bit architecture
- No MMU
- 32 interrupts per core
- 3 hardware timers per core

### Memory

- 384 KB internal SRAM
- 192 KB DRAM (0x3FC88000 - 0x3FCB8000)
- 256 KB IRAM (0x40080000 - 0x400C0000)
- 16 MB external SPI flash
- 8 MB external PSRAM (pseudo-static RAM)
- PSRAM mapped via cache at 0x3F800000

### Peripherals

- 3x UART
- 3x SPI
- 2x I2C
- 2x I2S
- 1x SDIO
- 1x RMT
- 1x LEDC (PWM)
- 1x MCPWM
- 1x USB OTG
- 1x USB Serial/JTAG
- 2x TWAI (CAN)
- ADC/DAC
- Touch sensor
- Temperature sensor

### GPIO

- 45 programmable GPIOs
- Configurable pull-up/down
- Interrupt support
- RTC GPIO subset

### Clock

- 40 MHz crystal
- PLL for 240/160/80 MHz
- RTC slow/fast clocks
- APB at 80 MHz

## Boot Process

1. ROM bootloader executes from mask ROM
2. Checks boot pins (GPIO0, GPIO46)
3. Loads binary from flash offset 0x0
4. Validates image signature
5. Jumps to entry point

## Flash Layout

```
0x000000 - 0x010000  Bootloader (64 KB)
0x010000 - 0x020000  Partition table (64 KB)
0x020000 - ...       Application
```

BareOS uses a flat binary at offset 0x0 for direct boot.

## Memory Map for BareOS

```
IRAM (0x40080000 - 0x400C0000):
  .text, .rodata, .vector_table

DRAM (0x3FC88000 - 0x3FCB8000):
  .data, .bss, heap (32 KB), stacks

PSRAM (0x3F800000 - 0x40000000):
  Mapped via cache, used for large allocations
```

### Linker Memory Regions

```
MEMORY {
    iram0_0_seg (rx)  : ORIGIN = 0x40080000, LENGTH = 0x40000
    dram0_0_seg (rwx) : ORIGIN = 0x3FC88000, LENGTH = 0x30000
    psram_seg (rw)    : ORIGIN = 0x3F800000, LENGTH = 0x800000
}
```

## UART Console

- UART0: GPIO1 (TX), GPIO3 (RX)
- Default: 115200 8N1
- Used for kernel console and shell

## Timer

- System timer: 52-bit at 16 MHz (SYSTIMER)
- Core timers: CCOUNT/CCOMPARE at CPU frequency
- Used for scheduler tick (1000 Hz)

## Interrupt Controller

- 32 interrupts per core
- Priority levels 1-7
- NMI at level 7
- Level-triggered and edge-triggered

## PSRAM Initialization Sequence

1. Enable PSRAM clock in PCR
2. Reset SPI1 controller
3. Read PSRAM ID (0x5D for ESP-PSRAM64, 0x26/0x27 for others)
4. Send reset commands (0x66, 0x99)
5. Configure SPI1 cache control register
6. Enable cache in SYSCON