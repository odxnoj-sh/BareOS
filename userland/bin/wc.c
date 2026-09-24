#include <libc.h>

int main(int argc, char **argv) {
    int show_lines = 1, show_words = 1, show_chars = 1;
    int file_idx = 1;

    if (argc > 1 && argv[1][0] == '-') {
        show_lines = show_words = show_chars = 0;
        for (char *p = argv[1] + 1; *p; p++) {
            if (*p == 'l') show_lines = 1;
            else if (*p == 'w') show_words = 1;
            else if (*p == 'c') show_chars = 1;
        }
        file_idx = 2;
    }

    long total_lines = 0, total_words = 0, total_chars = 0;

    for (int i = file_idx; i < argc; i++) {
        FILE *fp = fopen(argv[i], "r");
        if (!fp) {
            printf("wc: cannot open '%s'\n", argv[i]);
            return 1;
        }
        long lines = 0, words = 0, chars = 0;
        int in_word = 0;
        int c;
        while ((c = fgetc(fp)) != EOF) {
            chars++;
            if (c == '\n') lines++;
            if (isspace(c)) in_word = 0;
            else if (!in_word) { in_word = 1; words++; }
        }
        if (show_lines) printf("%7ld ", lines);
        if (show_words) printf("%7ld ", words);
        if (show_chars) printf("%7ld ", chars);
        printf("%s\n", argv[i]);
        fclose(fp);
        total_lines += lines;
        total_words += words;
        total_chars += chars;
    }

    if (argc - file_idx > 1) {
        if (show_lines) printf("%7ld ", total_lines);
        if (show_words) printf("%7ld ", total_words);
        if (show_chars) printf("%7ld ", total_chars);
        printf("total\n");
    }
    return 0;
}