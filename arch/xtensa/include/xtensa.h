#ifndef XTENSA_H
#define XTENSA_H

#include <stdint.h>

#define XTENSA_PSR_INTLEVEL_MASK 0x0000000F
#define XTENSA_PSR_INTLEVEL_SHIFT 0
#define XTENSA_PSR_EXCM_MASK 0x00000010
#define XTENSA_PSR_UM_MASK 0x00000020
#define XTENSA_PSR_RING_MASK 0x000000C0
#define XTENSA_PSR_OWB_MASK 0x00000F00
#define XTENSA_PSR_WOE_MASK 0x00001000
#define XTENSA_PSR_CALLINC_MASK 0x0000E000

#define XTENSA_CPUID_MANUF_MASK 0x000000FF
#define XTENSA_CPUID_PROCESSOR_ID_MASK 0x0000FF00
#define XTENSA_CPUID_REVISION_MASK 0x00FF0000
#define XTENSA_CPUID_VERSION_MASK 0xFF000000

#define XCHAL_INTENABLE_EXCM_LEVEL 0x00000010

#define EXCCAUSE_ILLEGAL_INSTRUCTION 0
#define EXCCAUSE_SYSCALL 1
#define EXCCAUSE_INSTRUCTION_FETCH_ERROR 2
#define EXCCAUSE_LOAD_STORE_ERROR 3
#define EXCCAUSE_LEVEL1_INTERRUPT 4
#define EXCCAUSE_ALLOCA 5
#define EXCCAUSE_INTEGER_DIVIDE_BY_ZERO 6
#define EXCCAUSE_SPECULATION 7
#define EXCCAUSE_PRIVILEGED 8
#define EXCCAUSE_UNALIGNED 9
#define EXCCAUSE_ITLB_MISS 16
#define EXCCAUSE_ITLB_MULTIHIT 17
#define EXCCAUSE_ITLB_PRIVILEGE 18
#define EXCCAUSE_ITLB_SIZE_RESTRICTION 19
#define EXCCAUSE_FETCH_CACHE_ATTRIBUTE 20
#define EXCCAUSE_DTLB_MISS 24
#define EXCCAUSE_DTLB_MULTIHIT 25
#define EXCCAUSE_DTLB_PRIVILEGE 26
#define EXCCAUSE_DTLB_SIZE_RESTRICTION 27
#define EXCCAUSE_LOAD_CACHE_ATTRIBUTE 28
#define EXCCAUSE_STORE_CACHE_ATTRIBUTE 29

extern uint32_t read_psr(void);
extern void write_psr(uint32_t val);
extern uint32_t read_epsr(void);
extern uint32_t read_epc(void);
extern void write_epc(uint32_t val);
extern uint32_t read_exccause(void);
extern uint32_t read_excvaddr(void);
extern uint32_t read_intenable(void);
extern void write_intenable(uint32_t val);
extern void write_intclear(uint32_t val);
extern uint32_t read_ccount(void);
extern void write_ccount(uint32_t val);
extern uint32_t read_ccompare0(void);
extern uint32_t read_ccompare1(void);
extern void write_ccompare0(uint32_t val);
extern void write_ccompare1(uint32_t val);
extern uint32_t read_sar(void);
extern void write_sar(uint32_t val);
extern void rsync(void);
extern void esync(void);
extern void dsync(void);
extern void isync(void);
extern void memw(void);
extern void waiti(int level);

static inline uint32_t read_ccompare(int n) {
    switch (n) {
        case 0: return read_ccompare0();
        case 1: return read_ccompare1();
    }
    return 0;
}

static inline void write_ccompare(int n, uint32_t val) {
    switch (n) {
        case 0: write_ccompare0(val); break;
        case 1: write_ccompare1(val); break;
    }
}

#endif