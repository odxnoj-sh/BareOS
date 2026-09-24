#include <libc.h>

int main(int argc, char **argv) {
    int lines = 10;
    int file_idx = 1;

    if (argc > 1 && argv[1][0] == '-') {
        lines = atoi(argv[1] + 1);
        file_idx = 2;
    }

    if (file_idx >= argc) {
        return 0;
    }

    for (int i = file_idx; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (!fp) {
            printf("tail: cannot open '%s'\n", argv[i]);
            return 1;
        }
        char buffer[1024][256];
        int count = 0;
        while (fgets(buffer[count % lines], sizeof(buffer[0]), fp)) {
            count++;
        }
        int start = count > lines ? count % lines : 0;
        for (int j = 0; j < lines && j < count; j++) {
            fputs(buffer[(start + j) % lines], stdout);
        }
        fclose(fp);
    }
    return 0;
}