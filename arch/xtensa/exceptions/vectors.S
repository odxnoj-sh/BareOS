.section .vector_table, "ax"
.align 4

.global _vector_table
_vector_table:

    .macro EXC_VECTOR num
    .align 4
    exc_vec_\num:
        movi a2, \num
        call0 _exception_entry
        j exc_vec_\num
    .endm

    EXC_VECTOR 0
    EXC_VECTOR 1
    EXC_VECTOR 2
    EXC_VECTOR 3
    EXC_VECTOR 4
    EXC_VECTOR 5
    EXC_VECTOR 6
    EXC_VECTOR 7
    EXC_VECTOR 8
    EXC_VECTOR 9
    EXC_VECTOR 10
    EXC_VECTOR 11
    EXC_VECTOR 12
    EXC_VECTOR 13
    EXC_VECTOR 14
    EXC_VECTOR 15
    EXC_VECTOR 16
    EXC_VECTOR 17
    EXC_VECTOR 18
    EXC_VECTOR 19
    EXC_VECTOR 20
    EXC_VECTOR 21
    EXC_VECTOR 22
    EXC_VECTOR 23
    EXC_VECTOR 24
    EXC_VECTOR 25
    EXC_VECTOR 26
    EXC_VECTOR 27
    EXC_VECTOR 28
    EXC_VECTOR 29
    EXC_VECTOR 30
    EXC_VECTOR 31

    .macro INT_VECTOR num
    .align 4
    int_vec_\num:
        call0 _interrupt_entry
        j int_vec_\num
    .endm

    INT_VECTOR 0
    INT_VECTOR 1
    INT_VECTOR 2
    INT_VECTOR 3
    INT_VECTOR 4
    INT_VECTOR 5
    INT_VECTOR 6
    INT_VECTOR 7
    INT_VECTOR 8
    INT_VECTOR 9
    INT_VECTOR 10
    INT_VECTOR 11
    INT_VECTOR 12
    INT_VECTOR 13
    INT_VECTOR 14
    INT_VECTOR 15
    INT_VECTOR 16
    INT_VECTOR 17
    INT_VECTOR 18
    INT_VECTOR 19
    INT_VECTOR 20
    INT_VECTOR 21
    INT_VECTOR 22
    INT_VECTOR 23
    INT_VECTOR 24
    INT_VECTOR 25
    INT_VECTOR 26
    INT_VECTOR 27
    INT_VECTOR 28
    INT_VECTOR 29
    INT_VECTOR 30
    INT_VECTOR 31

.size _vector_table, . - _vector_table

.section .text, "ax"
.align 4

.global _exception_entry
_exception_entry:
    addi a1, a1, -0x60
    s32i a0, a1, 0
    s32i a2, a1, 4
    s32i a3, a1, 8
    s32i a4, a1, 12
    s32i a5, a1, 16
    s32i a6, a1, 20
    s32i a7, a1, 24
    s32i a8, a1, 28
    s32i a9, a1, 32
    s32i a10, a1, 36
    s32i a11, a1, 40
    s32i a12, a1, 44
    s32i a13, a1, 48
    s32i a14, a1, 52
    s32i a15, a1, 56

    rsr.sar a0
    s32i a0, a1, 60

    rsr a0, 17
    s32i a0, a1, 64

    rsr.ps a0
    s32i a0, a1, 68

    mov a2, a1
    call0 trap_dispatch_exception

    l32i a15, a1, 56
    l32i a14, a1, 52
    l32i a13, a1, 48
    l32i a12, a1, 44
    l32i a11, a1, 40
    l32i a10, a1, 36
    l32i a9, a1, 32
    l32i a8, a1, 28
    l32i a7, a1, 24
    l32i a6, a1, 20
    l32i a5, a1, 16
    l32i a4, a1, 12
    l32i a3, a1, 8
    l32i a2, a1, 4
    l32i a0, a1, 0
    addi a1, a1, 0x60
    jx a0

.global _interrupt_entry
_interrupt_entry:
    addi a1, a1, -0x20
    s32i a0, a1, 0
    s32i a2, a1, 4
    s32i a3, a1, 8
    s32i a4, a1, 12
    s32i a5, a1, 16
    s32i a6, a1, 20
    s32i a7, a1, 24
    s32i a8, a1, 28
    s32i a9, a1, 32
    s32i a10, a1, 36
    s32i a11, a1, 40
    s32i a12, a1, 44
    s32i a13, a1, 48
    s32i a14, a1, 52
    s32i a15, a1, 56

    call0 trap_dispatch_interrupt

    l32i a15, a1, 56
    l32i a14, a1, 52
    l32i a13, a1, 48
    l32i a12, a1, 44
    l32i a11, a1, 40
    l32i a10, a1, 36
    l32i a9, a1, 32
    l32i a8, a1, 28
    l32i a7, a1, 24
    l32i a6, a1, 20
    l32i a5, a1, 16
    l32i a4, a1, 12
    l32i a3, a1, 8
    l32i a2, a1, 4
    l32i a0, a1, 0
    addi a1, a1, 0x20
    jx a0

.size _exception_entry, . - _exception_entry
.size _interrupt_entry, . - _interrupt_entry