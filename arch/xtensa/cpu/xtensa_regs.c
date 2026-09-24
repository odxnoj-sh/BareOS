#include <xtensa.h>

uint32_t read_psr(void) {
    uint32_t val;
    __asm__ volatile("rsr %0, psr" : "=r"(val));
    return val;
}

void write_psr(uint32_t val) {
    __asm__ volatile("wsr %0, psr" :: "r"(val));
}

uint32_t read_epsr(void) {
    uint32_t val;
    __asm__ volatile("rsr %0, epsr" : "=r"(val));
    return val;
}

uint32_t read_epc(void) {
    uint32_t val;
    __asm__ volatile("rsr %0, epc1" : "=r"(val));
    return val;
}

void write_epc(uint32_t val) {
    __asm__ volatile("wsr %0, epc1" :: "r"(val));
}

uint32_t read_exccause(void) {
    uint32_t val;
    __asm__ volatile("rsr %0, exccause" : "=r"(val));
    return val;
}

uint32_t read_excvaddr(void) {
    uint32_t val;
    __asm__ volatile("rsr %0, excvaddr" : "=r"(val));
    return val;
}

uint32_t read_intenable(void) {
    uint32_t val;
    __asm__ volatile("rsr %0, intenable" : "=r"(val));
    return val;
}

void write_intenable(uint32_t val) {
    __asm__ volatile("wsr %0, intenable" :: "r"(val));
}

void write_intclear(uint32_t val) {
    __asm__ volatile("wsr %0, intclear" :: "r"(val));
}

uint32_t read_ccount(void) {
    uint32_t val;
    __asm__ volatile("rsr %0, ccount" : "=r"(val));
    return val;
}

void write_ccount(uint32_t val) {
    __asm__ volatile("wsr %0, ccount" :: "r"(val));
}

uint32_t read_ccompare(int n) {
    uint32_t val = 0;
    switch (n) {
        case 0: __asm__ volatile("rsr %0, ccompare0" : "=r"(val)); break;
        case 1: __asm__ volatile("rsr %0, ccompare1" : "=r"(val)); break;
        case 2: __asm__ volatile("rsr %0, ccompare2" : "=r"(val)); break;
    }
    return val;
}

void write_ccompare(int n, uint32_t val) {
    switch (n) {
        case 0: __asm__ volatile("wsr %0, ccompare0" :: "r"(val)); break;
        case 1: __asm__ volatile("wsr %0, ccompare1" :: "r"(val)); break;
        case 2: __asm__ volatile("wsr %0, ccompare2" :: "r"(val)); break;
    }
}

uint32_t read_sar(void) {
    uint32_t val;
    __asm__ volatile("rsr %0, sar" : "=r"(val));
    return val;
}

void write_sar(uint32_t val) {
    __asm__ volatile("wsr %0, sar" :: "r"(val));
}

uint32_t read_threadptr(void) {
    uint32_t val;
    __asm__ volatile("rsr %0, threadptr" : "=r"(val));
    return val;
}

void write_threadptr(uint32_t val) {
    __asm__ volatile("wsr %0, threadptr" :: "r"(val));
}

void rsync(void) {
    __asm__ volatile("rsync");
}

void esync(void) {
    __asm__ volatile("esync");
}

void dsync(void) {
    __asm__ volatile("dsync");
}

void isync(void) {
    __asm__ volatile("isync");
}

void memw(void) {
    __asm__ volatile("memw");
}

void waiti(int level) {
    __asm__ volatile("waiti %0" :: "r"(level));
}