#include <kernel.h>
#include <xtensa.h>
#include <esp32s3_reg.h>
#include <string.h>

extern void uart_init(void);
extern void uart_puts(const char *);
extern void uart_puthex(uint32_t);
extern void uart_putdec(uint32_t);
extern void uart_putc(char);
extern int uart_getc(void);
extern void fs_init(void);
extern int chdir(const char *path);

#define EOF (-1)

static inline int isspace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\f' || c == '\v';
}

struct cpu_state cpu_states[2];
struct process *process_table[MAX_PROCESSES];
struct task *task_table[MAX_TASKS];
int current_pid = 1;
int current_tid = 1;

static char kernel_stack[2][KERNEL_STACK_SIZE];
static uint32_t kernel_stack_top[2];

static char shell_line[1024];
static char shell_cwd[256] = "/";

static int shell_parse_line(char *line, char **argv, int max_args) {
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

void shell_main(void) {
    shell_cwd[0] = '/'; shell_cwd[1] = 0;

    while (1) {
        uart_puts("bare@esp32:");
        uart_puts(shell_cwd);
        uart_puts("$ ");

        int c;
        int pos = 0;
        while ((c = uart_getc()) != '\n' && c != '\r' && c != EOF && pos < 1023) {
            if (c == '\b' || c == 127) {
                if (pos > 0) {
                    pos--;
                    uart_putc('\b');
                    uart_putc(' ');
                    uart_putc('\b');
                }
            } else if (c >= 32 && c < 127) {
                shell_line[pos++] = c;
                uart_putc(c);
            }
        }
        shell_line[pos] = 0;
        uart_puts("\n");

        if (!*shell_line) continue;

        int argc = shell_parse_line(shell_line, (char**)0x3FFE0000, 64);
        char **argv = (char**)0x3FFE0000;

        if (argc == 0) continue;

        if (strcmp(argv[0], "exit") == 0) {
            uart_puts("Exiting shell...\n");
            return;
        } else if (strcmp(argv[0], "cd") == 0) {
            const char *path = argc > 1 ? ((char**)0x3FFE0000)[1] : "/";
            if (chdir(path) < 0) {
                uart_puts("cd: ");
                uart_puts(path);
                uart_puts(": No such directory\n");
            }
        } else if (strcmp(argv[0], "pwd") == 0) {
            uart_puts(shell_cwd);
            uart_puts("\n");
        } else if (strcmp(argv[0], "echo") == 0) {
            for (int i = 1; i < argc; i++) {
                uart_puts(((char**)0x3FFE0000)[i]);
                if (i + 1 < argc) uart_puts(" ");
            }
            uart_puts("\n");
        } else if (strcmp(argv[0], "env") == 0) {
            for (char **env = (char**)0x3FFE0000; *env; env++) {
                uart_puts(*env);
                uart_puts("\n");
            }
        } else if (strcmp(argv[0], "help") == 0) {
            uart_puts("Built-in commands: cd, pwd, echo, env, help, exit\n");
        } else {
            uart_puts("bare: ");
            uart_puts((char*)argv[0]);
            uart_puts(": command not found\n");
        }
    }
}

void kernel_main(void) {
    int cpu = 0;
    cpu_states[cpu].current_task = NULL;
    cpu_states[cpu].task_list = NULL;
    cpu_states[cpu].task_count = 0;
    cpu_states[cpu].tick_count = 0;
    cpu_states[cpu].scheduler_lock = 0;
    cpu_states[cpu].nesting_level = 0;
    kernel_stack_top[cpu] = (uint32_t)&kernel_stack[cpu][KERNEL_STACK_SIZE];

    uart_init();
    uart_puts("BareOS boot\n");
    uart_puts("Architecture: Xtensa LX7\n");
    uart_puts("CPU: ESP32-S3\n");
    uart_puts("Cores: 2\n");
    uart_puts("Memory: 16MB Flash, 8MB PSRAM\n");
    uart_puts("Kernel version: 0.1.0\n");

    memory_init();
    uart_puts("Memory initialized\n");

    trap_init();
    uart_puts("Traps initialized\n");

    timer_init();
    uart_puts("Timer initialized\n");

    scheduler_init();
    uart_puts("Scheduler initialized\n");

    fs_init();
    uart_puts("Filesystem initialized\n");

    struct process *init_proc = process_create("init");
    struct task *init_task = task_create(init_proc, (void*)shell_main, PRIORITY_DEFAULT);
    task_wake(init_task);

    uart_puts("Starting scheduler\n");
    schedule();

    while (1) {
        waiti(0);
    }
}

void kernel_init(void) {
}

pid_t alloc_pid(void) {
    for (int i = 1; i < MAX_PROCESSES; i++) {
        if (!process_table[i]) {
            return i;
        }
    }
    return 0;
}

tid_t alloc_tid(void) {
    for (int i = 1; i < MAX_TASKS; i++) {
        if (!task_table[i]) {
            return i;
        }
    }
    return 0;
}

void free_pid(pid_t pid) {
    if (pid > 0 && pid < MAX_PROCESSES) {
        process_table[pid] = NULL;
    }
}

void free_tid(tid_t tid) {
    if (tid > 0 && tid < MAX_TASKS) {
        task_table[tid] = NULL;
    }
}