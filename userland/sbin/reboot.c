#include <libc.h>
#include <xtensa.h>

int main(int argc, char **argv) {
    printf("Rebooting...\n");
    void (*reset)(void) = (void(*)(void))0x40000400;
    reset();
    return 0;
}