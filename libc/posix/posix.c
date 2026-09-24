#include <libc.h>
#include <kernel.h>

extern struct file **get_fd_table(void);
extern int get_fd_count(void);

int open(const char *pathname, int flags, ...) {
    struct file *file = vfs_open(pathname, flags);
    if (!file) return -1;

    struct file **fd_table = get_fd_table();
    int *fd_count = (int *)get_fd_count();
    if (*fd_count >= 32) {
        vfs_close(file);
        return -1;
    }
    fd_table[*fd_count] = file;
    return (*fd_count)++;
}

int close(int fd) {
    struct file **fd_table = get_fd_table();
    if (fd < 0 || fd >= 32 || !fd_table[fd]) return -1;
    vfs_close(fd_table[fd]);
    fd_table[fd] = NULL;
    return 0;
}

ssize_t read(int fd, void *buf, size_t count) {
    struct file **fd_table = get_fd_table();
    if (fd < 0 || fd >= 32 || !fd_table[fd]) return -1;
    return vfs_read(fd_table[fd], buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    struct file **fd_table = get_fd_table();
    if (fd < 0 || fd >= 32 || !fd_table[fd]) return -1;
    return vfs_write(fd_table[fd], buf, count);
}

off_t lseek(int fd, off_t offset, int whence) {
    struct file **fd_table = get_fd_table();
    if (fd < 0 || fd >= 32 || !fd_table[fd]) return -1;
    return vfs_lseek(fd_table[fd], offset, whence);
}

int stat(const char *pathname, struct stat *statbuf) {
    return vfs_stat(pathname, statbuf);
}

int fstat(int fd, struct stat *statbuf) {
    struct file **fd_table = get_fd_table();
    if (fd < 0 || fd >= 32 || !fd_table[fd]) return -1;
    return vfs_stat(fd_table[fd]->path, statbuf);
}

int mkdir(const char *pathname, mode_t mode) {
    return vfs_mkdir(pathname, mode);
}

int rmdir(const char *pathname) {
    return vfs_rmdir(pathname);
}

int unlink(const char *pathname) {
    return vfs_unlink(pathname);
}

int rename(const char *oldpath, const char *newpath) {
    return vfs_rename(oldpath, newpath);
}

int chmod(const char *pathname, mode_t mode) {
    (void)pathname; (void)mode;
    return 0;
}

int chown(const char *pathname, uid_t owner, gid_t group) {
    (void)pathname; (void)owner; (void)group;
    return 0;
}

pid_t getpid(void) {
    extern pid_t current_pid;
    return current_pid;
}

pid_t getppid(void) {
    extern struct process *process_table[];
    pid_t pid = getpid();
    if (process_table[pid] && process_table[pid]->parent) {
        return process_table[pid]->parent->pid;
    }
    return 0;
}

uid_t getuid(void) { return 0; }
gid_t getgid(void) { return 0; }
int setuid(uid_t uid) { (void)uid; return 0; }
int setgid(gid_t gid) { (void)gid; return 0; }

int fork(void) {
    return -1;
}

int execve(const char *pathname, char *const argv[], char *const envp[]) {
    extern int do_execve(const char *, char *const [], char *const []);
    return do_execve(pathname, argv, envp);
}

int waitpid(pid_t pid, int *wstatus, int options) {
    extern int syscall_waitpid(pid_t, int *, int);
    return syscall_waitpid(pid, wstatus, options);
}

int pipe(int pipefd[2]) {
    extern int syscall_pipe(int *);
    return syscall_pipe(pipefd);
}

int dup(int oldfd) {
    return dup2(oldfd, 3);
}

int dup2(int oldfd, int newfd) {
    struct file **fd_table = get_fd_table();
    if (oldfd < 0 || oldfd >= 32 || !fd_table[oldfd]) return -1;
    if (newfd < 0 || newfd >= 32) return -1;
    if (fd_table[newfd]) close(newfd);
    fd_table[newfd] = fd_table[oldfd];
    return newfd;
}

int ioctl(int fd, unsigned long request, ...) {
    va_list ap;
    va_start(ap, request);
    void *arg = va_arg(ap, void *);
    va_end(ap);
    struct file **fd_table = get_fd_table();
    if (fd < 0 || fd >= 32 || !fd_table[fd]) return -1;
    return vfs_ioctl(fd_table[fd], request, arg);
}

char *getlogin(void) {
    return "bare";
}

int isatty(int fd) {
    return fd >= 0 && fd <= 2;
}

int execvp(const char *file, char *const argv[]) {
    char path[256];
    extern char **environ;
    if (strchr(file, '/')) {
        return execve(file, argv, environ);
    }
    char *paths[] = {"/bin", "/usr/bin", "/sbin", "/usr/sbin", NULL};
    for (int i = 0; paths[i]; i++) {
        snprintf(path, sizeof(path), "%s/%s", paths[i], file);
        execve(path, argv, environ);
    }
    return -1;
}

int putenv(char *string) {
    return setenv(strtok(string, "="), strtok(NULL, "="), 1);
}