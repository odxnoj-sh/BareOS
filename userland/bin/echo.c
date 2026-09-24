#include <libc.h>

int main(int argc, char **argv) {
    int newline = 1;
    for (int i = 1; i < argc; i++) {
        if (i == 1 && strcmp(argv[i], "-n") == 0) {
            newline = 0;
            continue;
        }
        printf("%s%s", argv[i], i + 1 < argc ? " " : "");
    }
    if (newline) printf("\n");
    return 0;
}