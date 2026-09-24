#include <xtensa.h>
#include <esp32s3_reg.h>

int cpu_get_id(void) {
    return 0;
}

void cpu_init(void) {
    cpu_get_id();

    write_intenable(0);
    write_intclear(0xFFFFFFFF);

    write_ccount(0);
    write_ccompare(0, 240000000 / 1000);

    uint32_t intena = read_intenable();
    intena |= (1 << 6);
    write_intenable(intena);
}

void cpu_idle(void) {
    waiti(0);
}