#include <libc.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: touch FILE...\n");
        return 1;
    }
    int ret = 0;
    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "a");
        if (!fp) {
            printf("touch: cannot touch '%s'\n", argv[i]);
            ret = 1;
        } else {
            fclose(fp);
        }
    }
    return ret;
}