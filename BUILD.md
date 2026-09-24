# Build Instructions

## Prerequisites

- Xtensa ESP32-S3 cross-compiler toolchain
- esptool.py for flashing
- Python 3 with pyserial for monitor

## Toolchain Setup

```bash
# Install ESP-IDF toolchain (includes Xtensa compiler)
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32s3
. ./export.sh
```

Or download prebuilt toolchain:
```bash
wget https://github.com/espressif/crosstool-NG/releases/download/esp-2022r1/xtensa-esp32s3-elf-gcc11_2_0-esp-2022r1-linux-amd64.tar.gz
tar xf xtensa-esp32s3-elf-gcc11_2_0-esp-2022r1-linux-amd64.tar.gz
export PATH=$PATH:$(pwd)/xtensa-esp32s3-elf/bin
```

## Building

```bash
cd bareos
make
```

Output artifacts:
- `build/bareos.elf` - ELF with symbols
- `build/bareos.bin` - Binary image for flashing
- `build/bareos.map` - Linker map

## Flashing

```bash
# Erase flash
make erase

# Flash binary
make flash

# Or manually:
esptool.py --chip esp32s3 --port /dev/ttyUSB0 --baud 921600 write_flash 0x0 build/bareos.bin
```

## Monitor

```bash
make monitor
# Or:
python3 -m serial.tools.miniterm /dev/ttyUSB0 115200
```

## Clean

```bash
make clean
```

## Debugging

```bash
# Disassembly
make disassemble

# Size analysis
make size
```