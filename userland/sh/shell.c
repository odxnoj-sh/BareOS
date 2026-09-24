#include <libc.h>

#define MAX_ARGS 32
#define MAX_LINE 512
#define MAX_HISTORY 8

static char history[MAX_HISTORY][MAX_LINE];
static int history_count = 0;
static int history_pos = 0;

int parse_line(char *line, char **argv, int max_args);
void execute_command(int argc, char **argv);
void builtin_cd(int argc, char **argv);
void builtin_echo(int argc, char **argv);
void builtin_env(void);
void builtin_export(int argc, char **argv);
void builtin_unset(int argc, char **argv);
void builtin_history(void);

void shell_init(void) {
    extern char *cwd;
    cwd[0] = '/'; cwd[1] = 0;
}

void shell_main(void) {
    char line[MAX_LINE];
    shell_init();

    while (1) {
        printf("bare@esp32:%s$ ", cwd);
        fflush(stdout);

        if (!fgets(line, MAX_LINE, stdin)) continue;
        line[strcspn(line, "\n")] = 0;

        if (!*line) continue;

        if (history_count < MAX_HISTORY) {
            strcpy(history[history_count++], line);
        } else {
            for (int i = 0; i < MAX_HISTORY - 1; i++) {
                strcpy(history[i], history[i + 1]);
            }
            strcpy(history[MAX_HISTORY - 1], line);
        }
        history_pos = history_count;

        char *argv[MAX_ARGS];
        int argc = parse_line(line, argv, MAX_ARGS);
        if (argc == 0) continue;

        if (strcmp(argv[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(argv[0], "cd") == 0) {
            builtin_cd(argc, argv);
        } else if (strcmp(argv[0], "pwd") == 0) {
            printf("%s\n", cwd);
        } else if (strcmp(argv[0], "echo") == 0) {
            builtin_echo(argc, argv);
        } else if (strcmp(argv[0], "env") == 0) {
            builtin_env();
        } else if (strcmp(argv[0], "export") == 0) {
            builtin_export(argc, argv);
        } else if (strcmp(argv[0], "unset") == 0) {
            builtin_unset(argc, argv);
        } else if (strcmp(argv[0], "history") == 0) {
            builtin_history();
        } else {
            execute_command(argc, argv);
        }
    }
}

int parse_line(char *line, char **argv, int max_args) {
    int argc = 0;
    char *p = line;

    while (*p && argc < max_args - 1) {
        while (*p && isspace(*p)) p++;
        if (!*p) break;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            argv[argc++] = p;
            while (*p && *p != quote) p++;
            if (*p) *p++ = 0;
        } else {
            argv[argc++] = p;
            while (*p && !isspace(*p)) p++;
            if (*p) *p++ = 0;
        }
    }
    argv[argc] = NULL;
    return argc;
}

void execute_command(int argc, char **argv) {
    (void)argc;
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        printf("bare: %s: command not found\n", argv[0]);
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
}

void builtin_cd(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : getenv("HOME");
    if (!path) path = "/";
    if (chdir(path) < 0) {
        printf("cd: %s: No such directory\n", path);
    }
}

void builtin_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        printf("%s%s", argv[i], i + 1 < argc ? " " : "");
    }
    printf("\n");
}

void builtin_env(void) {
    extern char **environ;
    for (char **env = environ; *env; env++) {
        printf("%s\n", *env);
    }
}

void builtin_export(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        putenv(argv[i]);
    }
}

void builtin_unset(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        unsetenv(argv[i]);
    }
}

void builtin_history(void) {
    for (int i = 0; i < history_count; i++) {
        printf("%d  %s\n", i + 1, history[i]);
    }
}