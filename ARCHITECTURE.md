# Architecture

## Memory Layout

```
0x40080000 - 0x400C0000  IRAM (256 KB) - Code execution
0x3FC88000 - 0x3FCB8000  DRAM (192 KB) - Data, heap, stack
0x3F800000 - 0x40000000  PSRAM (8 MB) - External memory (cache-mapped)
```

## Memory Regions

The kernel manages two physical memory regions:

1. **Internal DRAM** (0x3FC88000 - 0x3FCB8000): 192 KB
   - Kernel heap (32 KB)
   - Task stacks
   - BSS and data sections
   - Small allocations

2. **PSRAM** (0x3F800000 - 0x40000000): 8 MB
   - Large allocations
   - Filesystem buffers
   - Userland heap

## Kernel Structure

### Boot Process

1. ROM bootloader loads binary from flash
2. `_start` in startup.S sets up stack
3. BSS cleared, data copied
4. CPU initialized (PSR, interrupts, timers)
5. `kernel_main()` called
6. Subsystems initialized
7. Init process created
8. Scheduler started

### Scheduler

- Priority-based preemptive scheduler
- 32 priority levels (0-31)
- Per-CPU run queues
- Sleep queue with timer-based wakeup

### Memory Management

- No MMU - flat physical memory
- Two-region allocator: internal DRAM + PSRAM
- First-fit with splitting and coalescing
- Mutex-protected operations
- Per-region statistics
- Heap integrity checking

### Network Stack

- Layered design: socket API → UDP → IPv4 → ARP/link layer → network interface → hardware driver
- Loopback interface (lo0) with 127.0.0.1/8
- Packet buffer abstraction with prepend/append/trim
- Real checksums for IPv4, UDP, ICMP
- Socket integration with VFS file descriptor system
- ARP cache with timeout

### Process Model

- No fork() - no MMU for COW
- spawn/exec model for process creation
- Tasks belong to processes
- File descriptor table per process
- Per-process/task memory accounting

### System Calls

- SYSCALL instruction (EXCCAUSE=1)
- Arguments in a2-a7 registers
- Return value in a2

### Interrupts

- Level-based interrupt controller
- CCOUNT/CCOMPARE for timer
- Exception vectors in IRAM
- Nested interrupt support

## Multicore

- Core 0: Primary, runs kernel and tasks
- Core 1: Secondary, can run tasks
- Spinlocks for inter-core sync
- Per-CPU scheduler state