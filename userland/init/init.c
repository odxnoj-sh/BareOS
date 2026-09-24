#include <libc.h>

extern void shell_main(void);

void user_init(void) {
    shell_main();
}