#include <libc.h>

int main(int argc, char **argv) {
    if (argc < 2) return 1;

    if (strcmp(argv[1], "-f") == 0) {
        return access(argv[2], F_OK) == 0 ? 0 : 1;
    } else if (strcmp(argv[1], "-d") == 0) {
        struct stat st;
        return stat(argv[2], &st) == 0 && S_ISDIR(st.st_mode) ? 0 : 1;
    } else if (strcmp(argv[1], "-e") == 0) {
        return access(argv[2], F_OK) == 0 ? 0 : 1;
    } else if (strcmp(argv[1], "=") == 0) {
        return strcmp(argv[2], argv[3]) == 0 ? 0 : 1;
    } else if (strcmp(argv[1], "!=") == 0) {
        return strcmp(argv[2], argv[3]) != 0 ? 0 : 1;
    } else if (strcmp(argv[1], "-n") == 0) {
        return strlen(argv[2]) > 0 ? 0 : 1;
    } else if (strcmp(argv[1], "-z") == 0) {
        return strlen(argv[2]) == 0 ? 0 : 1;
    }
    return 1;
}