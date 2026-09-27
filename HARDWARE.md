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
- WiFi MAC/BB/RF (2.4 GHz)

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

## Network Stack Implementation

### Loopback Interface

- Interface name: `lo`
- IP address: 127.0.0.1/8
- MAC address: 00:00:00:00:00:00
- MTU: 1536 bytes
- Transmit function delivers packets back to receive path

### WiFi Interface

- Interface name: `wlan0`
- Hardware: ESP32-S3 WiFi MAC/BB/RF
- MAC address: Read from hardware registers (0x60036040/0x60036044)
- MTU: 1536 bytes
- DMA: 8 RX + 8 TX + 4 management descriptors in internal DRAM
- Interrupt: Level-based, handled by dedicated RX/mgmt tasks

### Protocol Support

- Ethernet: frame handling, ARP
- IPv4: header parsing, checksum, routing to local interfaces
- ICMP: Echo Request (type 8), Echo Reply (type 0)
- ARP: Request/Reply, cache with 5-minute timeout
- UDP: Header, checksum, port demultiplexing

### Socket API

- AF_INET / SOCK_DGRAM
- socket(), bind(), connect(), sendto(), recvfrom(), close()
- Integrated with VFS file descriptor system
- Port allocation: ephemeral (1024-65535) + explicit bind

## ESP32-S3 WiFi Hardware Registers

### WiFi MAC Base: 0x60036000

Key registers:
- WIFI_MAC_INT_RAW/ST/ENA/CLR (0x0000-0x000C)
- WIFI_MAC_TXCTRL/RXCTRL (0x0010/0x0020)
- WIFI_MAC_ADDR0/1 (0x0040/0x0044) - MAC address
- WIFI_MAC_BSSID0/1 (0x0048/0x004C) - BSSID
- WIFI_MAC_DMA_IN/OUT_CONF (0x0100/0x0110)
- WIFI_MAC_RX/TX_DMA_DESC (0x0120/0x0130)
- WIFI_MAC_MAILBOX (0x0200) - firmware mailbox
- WIFI_MAC_RATE_CTRL (0x0300) - rate control
- WIFI_MAC_CRYPTO_CTRL (0x0310) - crypto engine

### WiFi Power Base: 0x60036800

- WIFI_PWR_CLK_EN (0x0000) - clock enable
- WIFI_PWR_RST_EN (0x0004) - reset control
- WIFI_PWR_CTRL (0x0010) - power control

### WiFi Initialization Sequence

1. Enable WiFi clocks in PCR (0x60096130/0x60096134)
2. Enable power domain clocks (WIFI_PWR_CLK_EN)
3. Assert then deassert resets (WIFI_PWR_RST_EN)
4. Clear and disable interrupts
5. Read MAC address from WIFI_MAC_ADDR0/1
6. Initialize DMA descriptors in internal DRAM
7. Configure RX/TX DMA descriptor base addresses
8. Enable RX/TX in MAC control registers
9. Enable interrupts (RX_DONE, TX_DONE, RX_ERR, TX_ERR, MGMT_RX, MGMT_TX)

## WiFi Driver Architecture

### RX Path

1. Hardware receives frame → DMA writes to RX buffer
2. Interrupt triggered (WIFI_INT_RX_DONE)
3. ISR signals semaphore
4. RX task waits on semaphore
5. Task processes completed RX descriptor
6. Frame copied to packet buffer (pbuf)
7. pbuf passed to net_iface_input()
8. RX descriptor returned to hardware

### TX Path

1. net_iface_output() calls wifi_transmit()
2. Packet copied to next available TX buffer
3. TX descriptor updated with buffer pointer/length
4. Descriptor ownership transferred to DMA
5. Hardware transmits frame
6. TX_DONE interrupt clears descriptor ownership

### Management Frame Path

1. Dedicated management task handles association state machine
2. Builds management frames (probe request, auth request, assoc request)
3. Transmits via dedicated management DMA descriptors
4. Receives management frames via RX path (filtered by frame type)
5. Parses probe response, auth response, assoc response
6. State machine with timeouts and retries

### Buffer Management

- 8 RX descriptors + 8 TX descriptors + 4 management descriptors (16-byte aligned)
- 1536-byte buffers in internal DRAM
- Descriptors in circular linked list
- Owner bit tracks CPU/DMA ownership
- Management buffers for probe/auth/assoc frames

### Association State Machine

```
DOWN → INIT → UP → SCANNING → AUTHENTICATING → ASSOCIATING → ASSOCIATED
                    ↓              ↓              ↓
                  FAILED        FAILED         FAILED
```

States:
- DOWN: Driver not initialized
- INIT: Hardware initialization in progress
- UP: Hardware ready, not associated
- SCANNING: Sending probe requests, waiting for probe response
- AUTHENTICATING: Sent auth request, waiting for auth response
- ASSOCIATING: Sent assoc request, waiting for assoc response
- ASSOCIATED: Successfully associated, data path active
- FAILED: Association failed, cleanup required

Timeouts:
- Scan: 5 seconds
- Authentication: 3 seconds
- Association: 3 seconds
- Max retries: 3 per state

Supported Security Modes:
- Open System (implemented)
- WPA-PSK (stubbed, not implemented)

Credentials:
- SSID and password provided at runtime via wifi_set_credentials()
- No hardcoded credentials in source code
- Configuration via shell command or runtime API