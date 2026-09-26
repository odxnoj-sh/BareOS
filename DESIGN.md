# Design Decisions

## Why No MMU?

The ESP32-S3 has no MMU. This fundamentally changes OS design:
- No virtual memory → flat physical address space
- No copy-on-write → no fork()
- No memory protection → all tasks share memory
- Solution: spawn/exec model, cooperative memory safety

## Why Xtensa Assembly for Context Switch?

- Xtensa has unique register window (a0-a15, SAR)
- ABI requires specific save/restore sequence
- C compiler cannot generate correct context switch
- Assembly ensures correctness and performance

## Why Two-Region Heap Allocator?

- ESP32-S3 has internal DRAM (192 KB) and PSRAM (8 MB)
- Internal DRAM is faster and required for DMA
- PSRAM is slower but abundant
- Separate allocators avoid fragmentation in critical memory
- First-fit with coalescing is simple and effective

## Why Mutex-Protected Heap?

- BareOS has preemptive scheduler with multiple tasks
- Heap operations must be atomic
- Mutex allows blocking instead of spinning
- Avoids deadlock by not allocating in interrupt context

## Why Priority-Based Scheduler?

- Simple to implement and understand
- Deterministic latency for high-priority tasks
- 32 levels sufficient for embedded use
- No complex CFS or BFS needed

## Why VFS with Function Pointers?

- Clean abstraction for multiple filesystems
- devfs, ramfs, flashfs use same interface
- Function pointer table like BSD vnode ops
- Minimal overhead

## Why No Threads?

- Tasks are the unit of scheduling
- No pthreads API
- Processes contain tasks
- Simpler than 1:1 or M:N threading

## Why Minimal Libc?

- Freestanding environment
- Only functions needed by userland
- No bloat from unused functions
- BSD-style APIs where possible

## Why Flat Binary Image?

- ESP32-S3 ROM bootloader loads raw binary
- No ELF loading at runtime
- Simpler than U-Boot or custom bootloader
- Direct hardware control from reset

## Why 1000 Hz Timer Tick?

- Good balance of resolution and overhead
- 1 ms granularity for sleep/timers
- 240 MHz CPU handles tick easily
- Standard for many RTOS

## Why Spinlocks for Multicore?

- No mutex implementation for cross-core yet
- Spinlocks work for short critical sections
- Interrupts disabled during spinlock
- Simple and correct for now

## Why No Dynamic Linking?

- No MMU for shared libraries
- No ELF loader in kernel
- Static linking simpler and smaller
- All code in single binary

## Why C11?

- Modern C features (_Generic, _Static_assert, atomics)
- Better type safety
- Widely supported by GCC/Clang
- No C++ runtime overhead