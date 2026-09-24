#include <xtensa.h>
#include <esp32s3_reg.h>
#include <kernel.h>

static uint32_t tick_hz = 1000;

void timer_init(void) {
    write_ccount(0);
    write_ccompare(0, 240000000 / tick_hz);
    write_intenable(read_intenable() | (1 << 6));
}

void timer_set_freq(uint32_t hz) {
    tick_hz = hz;
    write_ccompare(0, read_ccount() + 240000000 / tick_hz);
}

uint32_t timer_get_ticks(void) {
    return cpu_states[0].tick_count;
}

uint32_t timer_get_freq(void) {
    return tick_hz;
}

void udelay(uint32_t us) {
    uint32_t start = read_ccount();
    uint32_t ticks_per_us = 240000000 / 1000000;
    while ((read_ccount() - start) < us * ticks_per_us) {
    }
}

void mdelay(uint32_t ms) {
    udelay(ms * 1000);
}