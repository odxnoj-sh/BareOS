#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>

#define KERNEL_VERSION_MAJOR 0
#define KERNEL_VERSION_MINOR 1
#define KERNEL_VERSION_PATCH 0

#define MAX_TASKS 64
#define MAX_PROCESSES 32
#define TASK_STACK_SIZE 4096
#define KERNEL_STACK_SIZE 2048

#define TASK_STATE_RUNNABLE 0
#define TASK_STATE_RUNNING 1
#define TASK_STATE_SLEEPING 2
#define TASK_STATE_BLOCKED 3
#define TASK_STATE_STOPPED 4
#define TASK_STATE_EXITED 5

#define PRIORITY_MAX 31
#define PRIORITY_DEFAULT 16
#define PRIORITY_MIN 0

#define SYSCALL_EXIT 0
#define SYSCALL_FORK 1
#define SYSCALL_EXEC 2
#define SYSCALL_WAIT 3
#define SYSCALL_READ 4
#define SYSCALL_WRITE 5
#define SYSCALL_OPEN 6
#define SYSCALL_CLOSE 7
#define SYSCALL_LSEEK 8
#define SYSCALL_STAT 9
#define SYSCALL_MKDIR 10
#define SYSCALL_RMDIR 11
#define SYSCALL_UNLINK 12
#define SYSCALL_RENAME 13
#define SYSCALL_GETPID 14
#define SYSCALL_GETPPID 15
#define SYSCALL_GETUID 16
#define SYSCALL_GETGID 17
#define SYSCALL_SLEEP 18
#define SYSCALL_YIELD 19
#define SYSCALL_IOCTL 20
#define SYSCALL_PIPE 21
#define SYSCALL_DUP 22
#define SYSCALL_DUP2 23
#define SYSCALL_GETTIME 24
#define SYSCALL_SETTIME 25
#define SYSCALL_KILL 26
#define SYSCALL_SIGNAL 27
#define SYSCALL_SIGACTION 28
#define SYSCALL_MMAP 29
#define SYSCALL_MUNMAP 30
#define SYSCALL_PREAD 31
#define SYSCALL_PWRITE 32
#define SYSCALL_CHDIR 33
#define SYSCALL_GETCWD 34
#define SYSCALL_FSTAT 35

typedef int pid_t;
typedef int tid_t;
typedef unsigned int uid_t;
typedef unsigned int gid_t;
typedef long off_t;
typedef int ssize_t;
typedef unsigned int mode_t;
typedef unsigned int dev_t;
typedef long time_t;
typedef long clock_t;

struct stat;

struct task_context {
    uint32_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint32_t a8, a9, a10, a11, a12, a13, a14, a15;
    uint32_t sar;
    uint32_t pc;
    uint32_t ps;
};

struct task {
    tid_t tid;
    pid_t pid;
    uint8_t state;
    uint8_t priority;
    uint8_t cpu_affinity;
    uint8_t flags;
    struct task_context *context;
    void *stack_base;
    size_t stack_size;
    struct task *next;
    struct task *prev;
    struct process *process;
    uint32_t wake_time;
    int exit_code;
};

struct process {
    pid_t pid;
    pid_t ppid;
    uid_t uid;
    gid_t gid;
    char name[32];
    struct task *main_task;
    struct task *tasks;
    int task_count;
    struct file **fd_table;
    int fd_count;
    int fd_max;
    void *heap_start;
    void *heap_end;
    void *heap_brk;
    struct process *parent;
    struct process *children;
    struct process *sibling;
    int exit_code;
    uint8_t exited;
};

struct cpu_state {
    struct task *current_task;
    struct task *idle_task;
    struct task *task_list;
    int task_count;
    uint32_t tick_count;
    uint32_t scheduler_lock;
    int nesting_level;
};

extern struct cpu_state cpu_states[2];
extern struct process *process_table[MAX_PROCESSES];
extern struct task *task_table[MAX_TASKS];
extern int current_pid;
extern int current_tid;

void kernel_main(void);
void kernel_init(void);
void scheduler_init(void);
void memory_init(void);
void trap_init(void);
void timer_init(void);
void uart_init(void);

pid_t alloc_pid(void);
tid_t alloc_tid(void);
void free_pid(pid_t pid);
void free_tid(tid_t tid);

struct process *process_create(const char *name);
void process_destroy(struct process *proc);
struct task *task_create(struct process *proc, void (*entry)(void), uint8_t priority);
void task_destroy(struct task *task);
void task_yield(void);
void task_sleep(uint32_t ticks);
void task_wake(struct task *task);
void schedule(void);
void scheduler_tick(void);

void *kmalloc(size_t size);
void *kzalloc(size_t size);
void kfree(void *ptr);
void *kmalloc_aligned(size_t size, size_t align);

void syscall_handler(struct task_context *ctx);

void syscall_exit(int status);
int syscall_fork(void);
int syscall_exec(const char *path, char *const argv[], char *const envp[]);
int syscall_waitpid(pid_t pid, int *status, int options);
void syscall_sleep(unsigned int seconds);
int syscall_kill(pid_t pid, int sig);
int syscall_pipe(int *pipefd);
int syscall_pread(int fd, void *buf, size_t count, off_t offset);
int syscall_pwrite(int fd, const void *buf, size_t count, off_t offset);
int syscall_mkdir(const char *path, mode_t mode);
int syscall_rmdir(const char *path);
int syscall_unlink(const char *path);
int syscall_rename(const char *oldpath, const char *newpath);
int syscall_chdir(const char *path);
int syscall_getcwd(char *buf, size_t size);
int syscall_fstat(int fd, struct stat *st);
int syscall_dup(int oldfd);
int syscall_dup2(int oldfd, int newfd);
int do_execve(const char *path, char *const argv[], char *const envp[]);

#endif