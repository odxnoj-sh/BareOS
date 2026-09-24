#include <libc.h>

int main(int argc, char **argv) {
    printf("\033[2J\033[H");
    return 0;
}