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

- Loopback interface (127.0.0.1/8) - fully functional
- WiFi interface (wlan0) - driver implemented with association
- No physical network hardware driver beyond WiFi
- No Ethernet driver
- IPv4 only (no IPv6)
- UDP only (no TCP)
- No DHCP client
- No DNS resolver
- No routing between interfaces
- No firewall or packet filtering
- No packet capture/debugging interface

## Network Stack Specific

- ARP cache limited to 16 entries
- No ARP persistence across reboots
- No ICMP error message generation (only echo)
- No IP fragmentation/reassembly
- No multicast or broadcast support
- Socket receive buffer limited by packet buffer size
- No socket options (SO_REUSEADDR, etc.)

## WiFi Driver Specific

- Open System authentication only (no WPA/WPA2/WPA3)
- No WiFi scanning UI
- No power management
- No coexistence with Bluetooth
- MAC address read from registers, eFuse access not implemented
- Static IP configuration only (no DHCP)
- No regulatory domain support
- No DFS/radar detection
- Association retries limited to 3 attempts
- No roaming support
- No 802.11n/ac features (HT/VHT)
- Single antenna (no MIMO)
- Management frame handling simplified (no protection against replay)

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
- WiFi: driver implemented with open authentication
- No ADC, DAC, PWM, LEDC, RMT, SDIO, USB, CAN

## Shell

- No command history persistence
- No tab completion
- No job control (bg, fg, jobs builtins are stubs)
- No aliases
- Limited line editing

## Userland

- Minimal command set (25+ commands)
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
2. Add WiFi WPA/WPA2/WPA3 authentication
3. Add DHCP client
4. Add DNS resolver
5. Complete multicore SMP support
6. Implement remaining device drivers
7. Add gdb stub for debugging
8. Implement power management
9. Add more userland utilities
10. Improve shell with history, completion, job control
11. Add dynamic module loading
12. Implement proper signal handling with sigaction
13. Add TCP protocol support
14. Add IPv6 support
15. Add WiFi scanning and regulatory domain support