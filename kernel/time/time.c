#include <kernel.h>
#include <xtensa.h>

static uint32_t system_ticks = 0;
static uint32_t tick_hz = 1000;

void time_init(void) {
    system_ticks = 0;
}

void time_tick(void) {
    system_ticks++;
}

uint32_t time_get_ticks(void) {
    return system_ticks;
}

uint32_t time_get_hz(void) {
    return tick_hz;
}

time_t time_get_time(void) {
    return system_ticks / tick_hz;
}

void time_set_time(time_t t) {
    system_ticks = t * tick_hz;
}