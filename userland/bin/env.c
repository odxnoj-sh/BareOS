#include <libc.h>

extern char **environ;

int main(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        if (strchr(argv[i], '=')) {
            putenv(argv[i]);
        } else {
            for (char **env = environ; *env; env++) {
                printf("%s\n", *env);
            }
            return 0;
        }
    }
    for (char **env = environ; *env; env++) {
        printf("%s\n", *env);
    }
    return 0;
}