#include <libc.h>
#include <xtensa.h>

int main(int argc, char **argv) {
    printf("Halting...\n");
    while (1) {
        waiti(0);
    }
    return 0;
}