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
extern void user_init(void);
extern void flash_init(void);
extern int flashfs_mount(const char *source, const char *target, int flags, void *data);

struct cpu_state cpu_states[2];
struct process *process_table[MAX_PROCESSES];
struct task *task_table[MAX_TASKS];
int current_pid = 1;
int current_tid = 1;

static char kernel_stack[2][KERNEL_STACK_SIZE];
static uint32_t kernel_stack_top[2];

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

    flash_init();
    uart_puts("Flash initialized\n");

    flashfs_mount(NULL, "/storage", 0, NULL);
    uart_puts("Flash filesystem mounted\n");

    scheduler_init();
    uart_puts("Scheduler initialized\n");

    fs_init();
    uart_puts("Filesystem initialized\n");

    struct process *init_proc = process_create("init");
    struct task *init_task = task_create(init_proc, (void*)user_init, PRIORITY_DEFAULT);
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

extern void user_init(void);