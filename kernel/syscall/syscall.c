#include <kernel.h>
#include <xtensa.h>
#include <libc.h>
#include <memory.h>
#include <vfs.h>

void syscall_exit(int status);
int syscall_fork(void);
int syscall_exec(const char *path, char **argv, char **envp);
int syscall_waitpid(pid_t pid, int *status, int options);
void syscall_sleep(unsigned int seconds);
int syscall_kill(pid_t pid, int sig);
int syscall_pipe(int *pipefd);
int do_execve(const char *path, char **argv, char **envp);

struct file *get_file(int fd) {
    struct task *current = cpu_states[0].current_task;
    if (!current || !current->process) return NULL;
    struct file **fd_table = current->process->fd_table;
    if (fd < 0 || fd >= current->process->fd_count || !fd_table[fd]) return NULL;
    return fd_table[fd];
}

void syscall_handler(struct task_context *ctx) {
    int syscall_num = ctx->a2;
    int ret = -1;

    switch (syscall_num) {
        case SYSCALL_EXIT: {
            int status = ctx->a3;
            syscall_exit(status);
            break;
        }
        case SYSCALL_FORK: {
            ret = syscall_fork();
            break;
        }
        case SYSCALL_EXEC: {
            const char *path = (const char *)ctx->a3;
            char **argv = (char **)ctx->a4;
            char **envp = (char **)ctx->a5;
            (void)path; (void)argv; (void)envp;
            ret = syscall_exec(path, argv, envp);
            break;
        }
        case SYSCALL_WAIT: {
            pid_t pid = ctx->a3;
            int *status = (int *)ctx->a4;
            int options = ctx->a5;
            (void)pid; (void)status; (void)options;
            ret = syscall_waitpid(pid, status, options);
            break;
        }
        case SYSCALL_READ: {
            int fd = ctx->a3;
            void *buf = (void *)ctx->a4;
            size_t count = ctx->a5;
            struct file *file = get_file(fd);
            if (file) ret = vfs_read(file, buf, count);
            break;
        }
        case SYSCALL_WRITE: {
            int fd = ctx->a3;
            const void *buf = (const void *)ctx->a4;
            size_t count = ctx->a5;
            struct file *file = get_file(fd);
            if (file) ret = vfs_write(file, buf, count);
            break;
        }
        case SYSCALL_OPEN: {
            const char *path = (const char *)ctx->a3;
            int flags = ctx->a4;
            struct file *file = vfs_open(path, flags);
            if (file) {
                struct task *current = cpu_states[0].current_task;
                struct file **fd_table = current->process->fd_table;
                if (current->process->fd_count < 32) {
                    fd_table[current->process->fd_count++] = file;
                    ret = current->process->fd_count - 1;
                } else {
                    ret = -1;
                }
            }
            break;
        }
        case SYSCALL_CLOSE: {
            int fd = ctx->a3;
            struct file *file = get_file(fd);
            if (file) {
                vfs_close(file);
                struct task *current = cpu_states[0].current_task;
                current->process->fd_table[fd] = NULL;
                ret = 0;
            }
            break;
        }
        case SYSCALL_LSEEK: {
            int fd = ctx->a3;
            off_t offset = ctx->a4;
            int whence = ctx->a5;
            struct file *file = get_file(fd);
            if (file) ret = vfs_lseek(file, offset, whence);
            break;
        }
        case SYSCALL_STAT: {
            const char *path = (const char *)ctx->a3;
            struct stat *st = (struct stat *)ctx->a4;
            ret = vfs_stat(path, st);
            break;
        }
        case SYSCALL_MKDIR: {
            const char *path = (const char *)ctx->a3;
            mode_t mode = ctx->a4;
            ret = vfs_mkdir(path, mode);
            break;
        }
        case SYSCALL_RMDIR: {
            const char *path = (const char *)ctx->a3;
            ret = vfs_rmdir(path);
            break;
        }
        case SYSCALL_UNLINK: {
            const char *path = (const char *)ctx->a3;
            ret = vfs_unlink(path);
            break;
        }
        case SYSCALL_RENAME: {
            const char *oldpath = (const char *)ctx->a3;
            const char *newpath = (const char *)ctx->a4;
            ret = vfs_rename(oldpath, newpath);
            break;
        }
        case SYSCALL_GETPID: {
            struct task *current = cpu_states[0].current_task;
            ret = current ? current->pid : 0;
            break;
        }
        case SYSCALL_GETPPID: {
            struct task *current = cpu_states[0].current_task;
            ret = current && current->process && current->process->parent ? current->process->parent->pid : 0;
            break;
        }
        case SYSCALL_SLEEP: {
            unsigned int seconds = ctx->a3;
            syscall_sleep(seconds);
            ret = 0;
            break;
        }
        case SYSCALL_YIELD: {
            task_yield();
            ret = 0;
            break;
        }
        case SYSCALL_IOCTL: {
            int fd = ctx->a3;
            int request = ctx->a4;
            void *arg = (void *)ctx->a5;
            struct file *file = get_file(fd);
            if (file) ret = vfs_ioctl(file, request, arg);
            break;
        }
        case SYSCALL_PIPE: {
            int *pipefd = (int *)ctx->a3;
            (void)pipefd;
            ret = syscall_pipe(pipefd);
            break;
        }
        case SYSCALL_DUP: {
            int oldfd = ctx->a3;
            struct file *file = get_file(oldfd);
            if (file) {
                struct task *current = cpu_states[0].current_task;
                for (int i = 0; i < 32; i++) {
                    if (!current->process->fd_table[i]) {
                        current->process->fd_table[i] = file;
                        ret = i;
                        break;
                    }
                }
            }
            break;
        }
        case SYSCALL_DUP2: {
            int oldfd = ctx->a3;
            int newfd = ctx->a4;
            struct file *file = get_file(oldfd);
            if (file && newfd >= 0 && newfd < 32) {
                struct task *current = cpu_states[0].current_task;
                if (current->process->fd_table[newfd]) vfs_close(current->process->fd_table[newfd]);
                current->process->fd_table[newfd] = file;
                ret = newfd;
            }
            break;
        }
        case SYSCALL_KILL: {
            pid_t pid = ctx->a3;
            int sig = ctx->a4;
            (void)pid; (void)sig;
            ret = syscall_kill(pid, sig);
            break;
        }
        default:
            ret = -1;
    }

    ctx->a2 = ret;
}

void syscall_exit(int status) {
    struct task *current = cpu_states[0].current_task;
    if (!current) return;
    current->exit_code = status;
    current->state = TASK_STATE_EXITED;
    if (current->process) current->process->exit_code = status;
    schedule();
}

int syscall_fork(void) {
    return -1;
}

int syscall_exec(const char *path, char **argv, char **envp) {
    (void)path; (void)argv; (void)envp;
    return do_execve(path, argv, envp);
}

int syscall_waitpid(pid_t pid, int *status, int options) {
    (void)pid; (void)status; (void)options;
    return -1;
}

void syscall_sleep(unsigned int seconds) {
    struct task *current = cpu_states[0].current_task;
    if (!current) return;
    current->wake_time = cpu_states[0].tick_count + seconds * 1000;
    current->state = TASK_STATE_SLEEPING;
    schedule();
}

int syscall_kill(pid_t pid, int sig) {
    (void)pid; (void)sig;
    return -1;
}

int syscall_pipe(int *pipefd) {
    (void)pipefd;
    return -1;
}

int do_execve(const char *path, char **argv, char **envp) {
    (void)path; (void)argv; (void)envp;
    return -1;
}

int access(const char *path, int mode) {
    (void)mode;
    struct stat st;
    return stat(path, &st);
}