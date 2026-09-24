#include <libc.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: sleep SECONDS\n");
        return 1;
    }
    unsigned int sec = atoi(argv[1]);
    sleep(sec);
    return 0;
}