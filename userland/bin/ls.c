#include <libc.h>

int main(int argc, char **argv) {
    const char *path = ".";
    int show_all = 0, long_format = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (char *p = argv[i] + 1; *p; p++) {
                if (*p == 'a') show_all = 1;
                else if (*p == 'l') long_format = 1;
            }
        } else {
            path = argv[i];
        }
    }

    struct file *dir = fopen(path, "r");
    if (!dir) {
        printf("ls: cannot access '%s': No such file or directory\n", path);
        return 1;
    }

    struct dirent ent;
    while (fread(&ent, sizeof(ent), 1, dir)) {
        if (!show_all && ent.d_name[0] == '.') continue;
        if (long_format) {
            struct stat st;
            char full[256];
            snprintf(full, sizeof(full), "%s/%s", path, ent.d_name);
            stat(full, &st);
            printf("%c%c%c%c%c%c%c%c%c%c %3d %s %s %8ld %s\n",
                (st.st_mode & S_IFDIR) ? 'd' : '-',
                st.st_mode & S_IRUSR ? 'r' : '-',
                st.st_mode & S_IWUSR ? 'w' : '-',
                st.st_mode & S_IXUSR ? 'x' : '-',
                st.st_mode & S_IRGRP ? 'r' : '-',
                st.st_mode & S_IWGRP ? 'w' : '-',
                st.st_mode & S_IXGRP ? 'x' : '-',
                st.st_mode & S_IROTH ? 'r' : '-',
                st.st_mode & S_IWOTH ? 'w' : '-',
                st.st_mode & S_IXOTH ? 'x' : '-',
                st.st_nlink, "user", "group", st.st_size, ent.d_name);
        } else {
            printf("%s  ", ent.d_name);
        }
    }
    if (!long_format) printf("\n");
    fclose(dir);
    return 0;
}