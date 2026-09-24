#include <libc.h>

int main(int argc, char **argv) {
    int lines = 10;
    int file_idx = 1;

    if (argc > 1 && argv[1][0] == '-') {
        lines = atoi(argv[1] + 1);
        file_idx = 2;
    }

    if (file_idx >= argc) {
        char buf[1024];
        for (int i = 0; i < lines && fgets(buf, sizeof(buf), stdin); i++) {
            fputs(buf, stdout);
        }
        return 0;
    }

    for (int i = file_idx; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (!fp) {
            printf("head: cannot open '%s'\n", argv[i]);
            return 1;
        }
        char buf[1024];
        for (int l = 0; l < lines && fgets(buf, sizeof(buf), fp); l++) {
            fputs(buf, stdout);
        }
        fclose(fp);
    }
    return 0;
}