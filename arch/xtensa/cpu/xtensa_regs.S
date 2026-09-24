#define PSR 0
#define EPSR 1
#define EPC1 17
#define EXCCAUSE 2
#define EXCVADDR 3
#define INTENABLE 4
#define INTSET 5
#define CCOUNT 232
#define CCOMPARE0 233
#define CCOMPARE1 234
#define SAR 16

.section .text, "ax"
.align 4

.global read_psr
read_psr:
    rsr a2, PSR
    mov a2, a2
    ret

.global write_psr
write_psr:
    wsr a2, PSR
    rsync
    ret

.global read_epsr
read_epsr:
    rsr a2, EPSR
    mov a2, a2
    ret

.global read_epc
read_epc:
    rsr a2, EPC1
    mov a2, a2
    ret

.global write_epc
write_epc:
    wsr a2, EPC1
    rsync
    ret

.global read_exccause
read_exccause:
    rsr a2, EXCCAUSE
    mov a2, a2
    ret

.global read_excvaddr
read_excvaddr:
    rsr a2, EXCVADDR
    mov a2, a2
    ret

.global read_intenable
read_intenable:
    rsr a2, INTENABLE
    mov a2, a2
    ret

.global write_intenable
write_intenable:
    wsr a2, INTENABLE
    rsync
    ret

.global write_intclear
write_intclear:
    ret

.global read_ccount
read_ccount:
    rsr a2, CCOUNT
    mov a2, a2
    ret

.global write_ccount
write_ccount:
    wsr a2, CCOUNT
    rsync
    ret

.global read_ccompare0
read_ccompare0:
    rsr a2, CCOMPARE0
    mov a2, a2
    ret

.global read_ccompare1
read_ccompare1:
    rsr a2, CCOMPARE1
    mov a2, a2
    ret

.global write_ccompare0
write_ccompare0:
    wsr a2, CCOMPARE0
    rsync
    ret

.global write_ccompare1
write_ccompare1:
    wsr a2, CCOMPARE1
    rsync
    ret

.global read_sar
read_sar:
    rsr a2, SAR
    mov a2, a2
    ret

.global write_sar
write_sar:
    wsr a2, SAR
    rsync
    ret

.global rsync
rsync:
    rsync
    ret

.global esync
esync:
    esync
    ret

.global dsync
dsync:
    dsync
    ret

.global isync
isync:
    isync
    ret

.global memw
memw:
    memw
    ret

.global waiti
waiti:
    waiti 0
    ret

.size read_psr, . - read_psr
.size write_psr, . - write_psr
.size read_epsr, . - read_epsr
.size read_epc, . - read_epc
.size write_epc, . - write_epc
.size read_exccause, . - read_exccause
.size read_excvaddr, . - read_excvaddr
.size read_intenable, . - read_intenable
.size write_intenable, . - write_intenable
.size write_intclear, . - write_intclear
.size read_ccount, . - read_ccount
.size write_ccount, . - write_ccount
.size read_ccompare0, . - read_ccompare0
.size read_ccompare1, . - read_ccompare1
.size write_ccompare0, . - write_ccompare0
.size write_ccompare1, . - write_ccompare1
.size read_sar, . - read_sar
.size write_sar, . - write_sar
.size rsync, . - rsync
.size esync, . - esync
.size dsync, . - dsync
.size isync, . - isync
.size memw, . - memw
.size waiti, . - waiti