#include <libc.h>
#include <kernel.h>

static time_t boot_time = 0;
static uint32_t boot_ticks = 0;

time_t time(time_t *tloc) {
    time_t t = boot_time + (cpu_states[0].tick_count - boot_ticks) / 1000;
    if (tloc) *tloc = t;
    return t;
}

struct tm *localtime(const time_t *timep) {
    static struct tm tm;
    time_t t = *timep;
    time_t days = t / 86400;
    time_t rem = t % 86400;
    tm.tm_hour = rem / 3600;
    tm.tm_min = (rem % 3600) / 60;
    tm.tm_sec = rem % 60;
    tm.tm_wday = (days + 4) % 7;
    tm.tm_year = 70;
    tm.tm_mon = 0;
    tm.tm_mday = 1;
    tm.tm_yday = 0;
    tm.tm_isdst = 0;
    return &tm;
}

char *ctime(const time_t *timep) {
    static char buf[26];
    struct tm *tm = localtime(timep);
    sprintf(buf, "%.3s %.3s %2d %02d:%02d:%02d %d\n",
        "Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec" + tm->tm_mon * 4,
        "Sun\0Mon\0Tue\0Wed\0Thu\0Fri\0Sat" + tm->tm_wday * 4,
        tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, 1900 + tm->tm_year);
    return buf;
}

clock_t clock(void) {
    return cpu_states[0].tick_count;
}

unsigned int sleep(unsigned int seconds) {
    extern void syscall_sleep(unsigned int);
    syscall_sleep(seconds);
    return 0;
}