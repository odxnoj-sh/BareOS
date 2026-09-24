#define XTENSA_PSR_INTLEVEL_MASK 0x0000000F

.section .vector_table, "ax"
.align 4

.global _start
_start:
    movi a1, _stack_top
    entry a1, 0x20

    call0 _bss_init
    call0 _data_init
    call0 _cpu_init
    call0 _kernel_main

_bss_init:
    movi a2, _bss_start
    movi a3, _bss_end
    beqz a3, 1f
    beq a2, a3, 1f
    movi.n a4, 0
2:  s32i.n a4, a2, 0
    addi.n a2, a2, 4
    bne a2, a3, 2b
1:  ret

_data_init:
    movi a2, _data_start
    movi a3, _data_end
    movi a4, _etext
    beq a2, a3, 1f
2:  l32i.n a5, a4, 0
    s32i.n a5, a2, 0
    addi.n a2, a2, 4
    addi.n a4, a4, 4
    bne a2, a3, 2b
1:  ret

_cpu_init:
    ret

.section .text, "ax"
.align 4

.global _kernel_main
_kernel_main:
    call0 kernel_main
1:  waiti 0
    j 1b

.size _kernel_main, . - _kernel_main