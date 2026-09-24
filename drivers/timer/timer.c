#include <esp32s3_reg.h>
#include <xtensa.h>

void timer_driver_init(void) {
}

void timer_set_alarm(int timer, uint64_t alarm) {
    (void)timer; (void)alarm;
}

uint64_t timer_get_counter(int timer) {
    (void)timer;
    return 0;
}

void timer_enable(int timer) {
    (void)timer;
}

void timer_disable(int timer) {
    (void)timer;
}