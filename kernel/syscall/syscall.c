#include <kernel.h>
#include <xtensa.h>
#include <libc.h>
#include <memory.h>
#include <vfs.h>
#include <exec/exec.h>

struct pipe {
    char buffer[4096];
    int read_pos;
    int write_pos;
    int readers;
    int writers;
    struct file *read_fd;
    struct file *write_fd;
};

ssize_t pipe_read(struct file *file, void *buf, size_t count);
ssize_t pipe_write(struct file *file, const void *buf, size_t count);
int pipe_close_read(struct file *file);
int pipe_close_write(struct file *file);

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
    struct task *current = cpu_states[0].current_task;
    if (!current || !current->process) return -1;

    struct process *new_proc = kzalloc(sizeof(struct process));
    if (!new_proc) return -1;

    new_proc->pid = alloc_pid();
    if (!new_proc->pid) {
        kfree(new_proc);
        return -1;
    }

    new_proc->ppid = current->process->pid;
    new_proc->uid = current->process->uid;
    new_proc->gid = current->process->gid;
    strncpy(new_proc->name, current->process->name, 31);
    new_proc->name[31] = 0;

    new_proc->fd_max = current->process->fd_max;
    new_proc->fd_table = kzalloc(sizeof(struct file *) * new_proc->fd_max);
    if (!new_proc->fd_table) {
        free_pid(new_proc->pid);
        kfree(new_proc);
        return -1;
    }
    new_proc->fd_count = current->process->fd_count;
    for (int i = 0; i < new_proc->fd_count; i++) {
        if (current->process->fd_table[i]) {
            new_proc->fd_table[i] = current->process->fd_table[i];
            new_proc->fd_table[i]->ref_count++;
        }
    }

    struct task *new_task = task_create(new_proc, (void *)current->context->pc, current->priority);
    if (!new_task) {
        for (int i = 0; i < new_proc->fd_count; i++) {
            if (new_proc->fd_table[i]) new_proc->fd_table[i]->ref_count--;
        }
        kfree(new_proc->fd_table);
        free_pid(new_proc->pid);
        kfree(new_proc);
        return -1;
    }

    memcpy(new_task->context, current->context, sizeof(struct task_context));
    new_task->context->a2 = 0;

    new_proc->parent = current->process;
    new_proc->sibling = current->process->children;
    current->process->children = new_proc;

    return new_proc->pid;
}

int syscall_exec(const char *path, char **argv, char **envp) {
    return do_execve(path, argv, envp);
}

int syscall_waitpid(pid_t pid, int *status, int options) {
    (void)options;
    struct task *current = cpu_states[0].current_task;
    if (!current || !current->process) return -1;

    struct process *child = current->process->children;
    struct process *prev = NULL;
    while (child) {
        if (pid == -1 || child->pid == pid) {
            if (child->exited) {
                if (status) *status = child->exit_code;
                if (prev) prev->sibling = child->sibling;
                else current->process->children = child->sibling;

                free_pid(child->pid);
                if (child->fd_table) kfree(child->fd_table);
                kfree(child);
                return pid == -1 ? child->pid : pid;
            }
        }
        prev = child;
        child = child->sibling;
    }

    current->state = TASK_STATE_BLOCKED;
    schedule();
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
    struct pipe *pipe = kzalloc(sizeof(struct pipe));
    if (!pipe) return -1;

    pipe->read_fd = kzalloc(sizeof(struct file));
    pipe->write_fd = kzalloc(sizeof(struct file));
    if (!pipe->read_fd || !pipe->write_fd) {
        kfree(pipe->read_fd);
        kfree(pipe->write_fd);
        kfree(pipe);
        return -1;
    }

    static struct file_operations pipe_read_fops = {
        .read = pipe_read,
        .close = pipe_close_read,
    };
    static struct file_operations pipe_write_fops = {
        .write = pipe_write,
        .close = pipe_close_write,
    };

    pipe->read_fd->fops = &pipe_read_fops;
    pipe->read_fd->private_data = pipe;
    pipe->read_fd->ref_count = 1;
    pipe->write_fd->fops = &pipe_write_fops;
    pipe->write_fd->private_data = pipe;
    pipe->write_fd->ref_count = 1;

    struct task *current = cpu_states[0].current_task;
    struct file **fd_table = current->process->fd_table;

    int read_fd = -1, write_fd = -1;
    for (int i = 0; i < 32; i++) {
        if (!fd_table[i]) {
            if (read_fd == -1) {
                read_fd = i;
                fd_table[i] = pipe->read_fd;
            } else if (write_fd == -1) {
                write_fd = i;
                fd_table[i] = pipe->write_fd;
                break;
            }
        }
    }

    if (read_fd == -1 || write_fd == -1) {
        kfree(pipe->read_fd);
        kfree(pipe->write_fd);
        kfree(pipe);
        return -1;
    }

    current->process->fd_count = (write_fd > read_fd) ? write_fd + 1 : read_fd + 1;
    pipefd[0] = read_fd;
    pipefd[1] = write_fd;
    return 0;
}

ssize_t pipe_read(struct file *file, void *buf, size_t count) {
    struct pipe *pipe = (struct pipe *)file->private_data;
    if (!pipe) return -1;

    while (pipe->read_pos == pipe->write_pos && pipe->writers > 0) {
        schedule();
    }

    if (pipe->read_pos == pipe->write_pos && pipe->writers == 0) {
        return 0;
    }

    int available = pipe->write_pos - pipe->read_pos;
    if (available > (int)count) available = count;
    if (available > 0) {
        memcpy(buf, pipe->buffer + pipe->read_pos, available);
        pipe->read_pos += available;
        if (pipe->read_pos == pipe->write_pos) {
            pipe->read_pos = pipe->write_pos = 0;
        }
    }
    return available;
}

ssize_t pipe_write(struct file *file, const void *buf, size_t count) {
    struct pipe *pipe = (struct pipe *)file->private_data;
    if (!pipe) return -1;

    while (pipe->write_pos - pipe->read_pos >= 4096 && pipe->readers > 0) {
        schedule();
    }

    if (pipe->readers == 0) return -1;

    int available = 4096 - (pipe->write_pos - pipe->read_pos);
    if (available > (int)count) available = count;
    if (available > 0) {
        memcpy(pipe->buffer + pipe->write_pos, buf, available);
        pipe->write_pos += available;
    }
    return available;
}

int pipe_close_read(struct file *file) {
    struct pipe *pipe = (struct pipe *)file->private_data;
    if (!pipe) return -1;
    pipe->readers--;
    if (pipe->readers == 0 && pipe->writers > 0) {
        schedule();
    }
    return 0;
}

int pipe_close_write(struct file *file) {
    struct pipe *pipe = (struct pipe *)file->private_data;
    if (!pipe) return -1;
    pipe->writers--;
    if (pipe->writers == 0 && pipe->readers > 0) {
        schedule();
    }
    return 0;
}