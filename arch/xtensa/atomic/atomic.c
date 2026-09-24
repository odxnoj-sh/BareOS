#include <xtensa.h>

int atomic_cmpxchg(volatile int *ptr, int old, int new) {
    int prev;
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);
    prev = *ptr;
    if (prev == old) *ptr = new;
    write_psr(psr);
    return prev;
}

void atomic_set(volatile int *ptr, int val) {
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);
    *ptr = val;
    write_psr(psr);
}

int atomic_get(volatile int *ptr) {
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);
    int val = *ptr;
    write_psr(psr);
    return val;
}

void atomic_add(volatile int *ptr, int val) {
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);
    *ptr += val;
    write_psr(psr);
}

void atomic_sub(volatile int *ptr, int val) {
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);
    *ptr -= val;
    write_psr(psr);
}

int atomic_inc(volatile int *ptr) {
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);
    int val = ++(*ptr);
    write_psr(psr);
    return val;
}

int atomic_dec(volatile int *ptr) {
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);
    int val = --(*ptr);
    write_psr(psr);
    return val;
}

int atomic_xchg(volatile int *ptr, int val) {
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);
    int old = *ptr;
    *ptr = val;
    write_psr(psr);
    return old;
}

unsigned long atomic_cmpxchg_ul(volatile unsigned long *ptr, unsigned long old, unsigned long new) {
    unsigned long prev;
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);
    prev = *ptr;
    if (prev == old) *ptr = new;
    write_psr(psr);
    return prev;
}