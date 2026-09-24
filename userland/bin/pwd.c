#include <libc.h>

int main(int argc, char **argv) {
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
    return 0;
}