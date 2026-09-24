# BareOS

BareOS is a bare-metal operating system for the ESP32-S3 microcontroller.

## Architecture

- CPU: Xtensa LX7 dual-core
- Clock: 240 MHz
- Flash: 16 MB
- PSRAM: 8 MB
- No MMU

## Features

- Preemptive multitasking scheduler
- Virtual filesystem (VFS) with ramfs and devfs
- Device nodes (/dev/console, /dev/null, /dev/zero, /dev/random, /dev/uart0)
- Unix-like system calls
- BSD-style userland utilities
- Interactive shell with job control

## Building

```bash
make
```

## Flashing

```bash
make flash
```

## Monitor

```bash
make monitor
```

## Directory Structure

```
bareos/
├── arch/xtensa/          # Architecture-specific code
├── kernel/               # Kernel core
├── drivers/              # Device drivers
├── fs/                   # Filesystem layer
├── libc/                 # C library
├── userland/             # User applications
├── include/              # Public headers
└── linker/               # Linker scripts
```