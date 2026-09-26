OUTPUT_ARCH(xtensa)
ENTRY(_start)

MEMORY
{
    iram0_0_seg (rx)  : ORIGIN = 0x40080000, LENGTH = 0x40000
    dram0_0_seg (rwx) : ORIGIN = 0x3FC88000, LENGTH = 0x30000
    psram_seg (rw)    : ORIGIN = 0x3F800000, LENGTH = 0x800000
}

SECTIONS
{
    . = 0x40080000;
    .text : ALIGN(4)
    {
        *(.vector_table)
        *(.text .text.* .gnu.linkonce.t.*)
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        . = ALIGN(4);
        _etext = .;
    } > iram0_0_seg

    .data : ALIGN(4)
    {
        _data_start = .;
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(4);
        _data_end = .;
    } > dram0_0_seg AT > iram0_0_seg

    .bss : ALIGN(4)
    {
        _bss_start = .;
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > dram0_0_seg

    .heap : ALIGN(4)
    {
        _heap_start = .;
        . = . + 0x8000;
        _heap_end = .;
    } > dram0_0_seg

    .stack : ALIGN(16)
    {
        _stack_bottom = .;
        . = . + 0x2000;
        _stack_top = .;
    } > dram0_0_seg

    _kernel_end = .;

    _dram_start = ORIGIN(dram0_0_seg);
    _dram_end = ORIGIN(dram0_0_seg) + LENGTH(dram0_0_seg);
    _psram_start = ORIGIN(psram_seg);
    _psram_end = ORIGIN(psram_seg) + LENGTH(psram_seg);
}