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

int htons(int hostshort) {
    return __builtin_bswap16(hostshort);
}

int htonl(int hostlong) {
    return __builtin_bswap32(hostlong);
}

int ntohs(int netshort) {
    return __builtin_bswap16(netshort);
}

int ntohl(int netlong) {
    return __builtin_bswap32(netlong);
}

int socket(int domain, int type, int protocol) {
    register long a0 __asm__("a0") = SYSCALL_SOCKET;
    register long a2 __asm__("a2") = domain;
    register long a3 __asm__("a3") = type;
    register long a4 __asm__("a4") = protocol;
    __asm__ volatile("syscall" : "+r"(a0) : "r"(a2), "r"(a3), "r"(a4) : "memory");
    return (int)a0;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    register long a0 __asm__("a0") = SYSCALL_BIND;
    register long a2 __asm__("a2") = sockfd;
    register long a3 __asm__("a3") = (long)addr;
    register long a4 __asm__("a4") = addrlen;
    __asm__ volatile("syscall" : "+r"(a0) : "r"(a2), "r"(a3), "r"(a4) : "memory");
    return (int)a0;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    register long a0 __asm__("a0") = SYSCALL_CONNECT;
    register long a2 __asm__("a2") = sockfd;
    register long a3 __asm__("a3") = (long)addr;
    register long a4 __asm__("a4") = addrlen;
    __asm__ volatile("syscall" : "+r"(a0) : "r"(a2), "r"(a3), "r"(a4) : "memory");
    return (int)a0;
}

int sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
    register long a0 __asm__("a0") = SYSCALL_SENDTO;
    register long a2 __asm__("a2") = sockfd;
    register long a3 __asm__("a3") = (long)buf;
    register long a4 __asm__("a4") = len;
    register long a5 __asm__("a5") = flags;
    register long a6 __asm__("a6") = (long)dest_addr;
    register long a7 __asm__("a7") = addrlen;
    __asm__ volatile("syscall" : "+r"(a0) : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7) : "memory");
    return (int)a0;
}

int recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
    register long a0 __asm__("a0") = SYSCALL_RECVFROM;
    register long a2 __asm__("a2") = sockfd;
    register long a3 __asm__("a3") = (long)buf;
    register long a4 __asm__("a4") = len;
    register long a5 __asm__("a5") = flags;
    register long a6 __asm__("a6") = (long)src_addr;
    register long a7 __asm__("a7") = (long)addrlen;
    __asm__ volatile("syscall" : "+r"(a0) : "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7) : "memory");
    return (int)a0;
}

int inet_pton(int af, const char *src, void *dst) {
    (void)af; (void)src; (void)dst;
    return -1;
}

char *inet_ntoa(struct in_addr in) {
    static char buf[16];
    uint32_t addr = __builtin_bswap32(in.s_addr);
    snprintf(buf, sizeof(buf), "%u.%u.%u.%u",
        (addr >> 24) & 0xFF, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF);
    return buf;
}