#include <libc.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        char buf[1024];
        while (fgets(buf, sizeof(buf), stdin)) {
            fputs(buf, stdout);
        }
        return 0;
    }

    for (int i = 1; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (!fp) {
            printf("cat: %s: No such file or directory\n", argv[i]);
            return 1;
        }
        char buf[1024];
        size_t n;
        while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) {
            fwrite(buf, 1, n, stdout);
        }
        fclose(fp);
    }
    return 0;
}