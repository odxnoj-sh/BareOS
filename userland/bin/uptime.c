#include <libc.h>
#include <kernel.h>

int main(int argc, char **argv) {
    uint32_t ticks = cpu_states[0].tick_count;
    uint32_t seconds = ticks / 1000;
    uint32_t days = seconds / 86400;
    seconds %= 86400;
    uint32_t hours = seconds / 3600;
    seconds %= 3600;
    uint32_t minutes = seconds / 60;
    seconds %= 60;

    printf("up ");
    if (days) printf("%ud ", days);
    if (hours || days) printf("%uh ", hours);
    printf("%um\n", minutes);
    return 0;
}