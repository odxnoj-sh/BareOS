#include <libc.h>

int main(int argc, char **argv) {
    time_t t = time(NULL);
    printf("%s", ctime(&t));
    return 0;
}