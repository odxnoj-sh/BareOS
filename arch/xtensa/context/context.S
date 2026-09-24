.section .text, "ax"
.align 4

.global context_switch
context_switch:
    movi a2, 0x20
    add a2, a1, a2
    s32i a2, a0, 0

    s32i a2, a0, 4
    s32i a3, a0, 8
    s32i a4, a0, 12
    s32i a5, a0, 16
    s32i a6, a0, 20
    s32i a7, a0, 24
    s32i a8, a0, 28
    s32i a9, a0, 32
    s32i a10, a0, 36
    s32i a11, a0, 40
    s32i a12, a0, 44
    s32i a13, a0, 48
    s32i a14, a0, 52
    s32i a15, a0, 56

    rsr.sar a2
    s32i a2, a0, 60

    movi a2, 0
    s32i a2, a0, 64

    rsr.ps a2
    s32i a2, a0, 68

    l32i a1, a3, 0

    l32i a2, a3, 4
    l32i a3, a3, 8
    l32i a4, a3, 12
    l32i a5, a3, 16
    l32i a6, a3, 20
    l32i a7, a3, 24
    l32i a8, a3, 28
    l32i a9, a3, 32
    l32i a10, a3, 36
    l32i a11, a3, 40
    l32i a12, a3, 44
    l32i a13, a3, 48
    l32i a14, a3, 52
    l32i a15, a3, 56

    l32i a2, a3, 60
    wsr.sar a2

    l32i a2, a3, 68
    wsr.ps a2
    rsync

    jx a2

.global context_save
context_save:
    movi a2, 0x20
    add a2, a1, a2
    s32i a2, a0, 0

    s32i a2, a0, 4
    s32i a3, a0, 8
    s32i a4, a0, 12
    s32i a5, a0, 16
    s32i a6, a0, 20
    s32i a7, a0, 24
    s32i a8, a0, 28
    s32i a9, a0, 32
    s32i a10, a0, 36
    s32i a11, a0, 40
    s32i a12, a0, 44
    s32i a13, a0, 48
    s32i a14, a0, 52
    s32i a15, a0, 56

    rsr.sar a2
    s32i a2, a0, 60

    movi a2, 0
    s32i a2, a0, 64

    rsr.ps a2
    s32i a2, a0, 68

    ret

.global context_restore
context_restore:
    l32i a1, a0, 0

    l32i a2, a0, 4
    l32i a3, a0, 8
    l32i a4, a0, 12
    l32i a5, a0, 16
    l32i a6, a0, 20
    l32i a7, a0, 24
    l32i a8, a0, 28
    l32i a9, a0, 32
    l32i a10, a0, 36
    l32i a11, a0, 40
    l32i a12, a0, 44
    l32i a13, a0, 48
    l32i a14, a0, 52
    l32i a15, a0, 56

    l32i a2, a0, 60
    wsr.sar a2

    l32i a2, a0, 68
    wsr.ps a2
    rsync

    jx a2

.size context_switch, . - context_switch
.size context_save, . - context_save
.size context_restore, . - context_restore