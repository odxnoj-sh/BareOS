# Limitations

## Memory Management

- No MMU - no virtual memory, no memory protection between tasks
- No fork() with copy-on-write - use spawn/exec instead
- No demand paging - all memory must be physically present
- No swap - limited to physical RAM (192 KB DRAM + 8 MB PSRAM)
- Internal DRAM heap is 32 KB - limited for kernel allocations
- PSRAM access is slower than internal DRAM (cache-mapped)
- No DMA-safe allocator - DMA requires internal memory
- No executable memory allocator - code must be in IRAM

## Process Model

- No traditional Unix process isolation
- All tasks share same address space
- No setuid/setgid - single user (root)
- No chroot or namespace isolation
- Signal handling is minimal (SIGKILL, SIGTERM, SIGINT, SIGCHLD only)

## Filesystem

- ramfs is volatile - data lost on reboot
- No persistent filesystem implemented yet
- No block device layer for flash
- No wear leveling for flash
- Limited to 64 KB ramfs data pool

## Networking

- No network stack implemented
- No WiFi driver
- No Ethernet driver
- No TCP/IP, UDP, DHCP, DNS

## Multicore

- Core 1 support is minimal
- No load balancing between cores
- No per-CPU data structures for all subsystems
- Spinlocks only for inter-core sync

## Device Drivers

- GPIO: basic only, no interrupt handling
- SPI: stub only
- I2C: stub only
- DMA: stub only
- Flash: stub only
- PSRAM: real implementation for memory mapping
- No ADC, DAC, PWM, LEDC, RMT, SDIO, USB, CAN

## Shell

- No command history persistence
- No tab completion
- No job control (bg, fg, jobs builtins are stubs)
- No aliases
- Limited line editing

## Userland

- Minimal command set (25 commands)
- No text editors (vi, nano)
- No scripting language (awk, sed, perl, python)
- No compiler toolchain on target
- No package manager

## Standards Compliance

- Not POSIX compliant
- Not BSD compatible
- Subset of libc functions
- No locale support
- No wide character support
- No pthreads

## Debugging

- No gdb stub
- No JTAG support configured
- Limited kernel logging
- No core dumps

## Power Management

- No sleep modes
- No DVFS
- No battery monitoring

## Security

- No encryption
- No secure boot
- No flash encryption
- No hardware random number generator usage

## Real-time

- No priority inheritance for mutexes
- No deadline scheduling
- Interrupt latency not characterized
- No rate-monotonic analysis

## Future Work

1. Implement persistent filesystem (littlefs or fatfs on flash)
2. Add WiFi driver and lwIP network stack
3. Complete multicore SMP support
4. Implement remaining device drivers
5. Add gdb stub for debugging
6. Implement power management
7. Add more userland utilities
8. Improve shell with history, completion, job control
9. Add dynamic module loading
10. Implement proper signal handling with sigaction