#ifndef LIBC_H
#define LIBC_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <vfs.h>

#define NULL ((void *)0)
#define EOF (-1)
#define BUFSIZ 1024
#define FILENAME_MAX 256
#define FOPEN_MAX 32

struct _FILE {
    int fd;
    int flags;
    char *buf;
    size_t buf_size;
    size_t buf_pos;
    size_t buf_len;
    int ungetc_buf;
    int ungetc_used;
};

typedef struct _FILE FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

struct tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
};

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0x40
#define O_TRUNC 0x200
#define O_APPEND 0x400

#define S_IFMT 0170000
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFBLK 0060000
#define S_IFIFO 0010000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001

#define DT_UNKNOWN 0
#define DT_REG 8
#define DT_DIR 4
#define DT_CHR 2
#define DT_BLK 6
#define DT_FIFO 1
#define DT_LNK 10
#define DT_SOCK 12

int open(const char *pathname, int flags, ...);
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);
off_t lseek(int fd, off_t offset, int whence);
int stat(const char *pathname, struct stat *statbuf);
int fstat(int fd, struct stat *statbuf);
int mkdir(const char *pathname, mode_t mode);
int rmdir(const char *pathname);
int unlink(const char *pathname);
int rename(const char *oldpath, const char *newpath);
int chmod(const char *pathname, mode_t mode);
int chown(const char *pathname, uid_t owner, gid_t group);

pid_t getpid(void);
pid_t getppid(void);
uid_t getuid(void);
gid_t getgid(void);
int setuid(uid_t uid);
int setgid(gid_t gid);

void *malloc(size_t size);
void free(void *ptr);
void *calloc(size_t nmemb, size_t size);
void *realloc(void *ptr, size_t size);

char *getcwd(char *buf, size_t size);
int chdir(const char *path);
extern char *cwd;

int fork(void);
int execve(const char *pathname, char *const argv[], char *const envp[]);
int execvp(const char *file, char *const argv[]);
int waitpid(pid_t pid, int *wstatus, int options);
void exit(int status);
void _exit(int status);

int kill(pid_t pid, int sig);
unsigned int sleep(unsigned int seconds);
int usleep(unsigned int microseconds);

time_t time(time_t *tloc);
struct tm *localtime(const time_t *timep);
char *ctime(const time_t *timep);

int pipe(int pipefd[2]);
int dup(int oldfd);
int dup2(int oldfd, int newfd);

int ioctl(int fd, unsigned long request, ...);

#define AF_INET 2
#define SOCK_DGRAM 2

typedef int socklen_t;

struct in_addr {
    uint32_t s_addr;
};

int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);
int recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

int htons(int hostshort);
int htonl(int hostlong);
int ntohs(int netshort);
int ntohl(int netlong);

int inet_pton(int af, const char *src, void *dst);
char *inet_ntoa(struct in_addr in);

int pipe(int pipefd[2]);

void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);
int memcmp(const void *s1, const void *s2, size_t n);
void *memchr(const void *s, int c, size_t n);

char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t n);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
size_t strlen(const char *s);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
char *strstr(const char *haystack, const char *needle);
char *strtok(char *str, const char *delim);
size_t strcspn(const char *s, const char *reject);

int isdigit(int c);
int isalpha(int c);
int isalnum(int c);
int isspace(int c);
int islower(int c);
int isupper(int c);
int tolower(int c);
int toupper(int c);

int atoi(const char *str);
long atol(const char *str);
long strtol(const char *str, char **endptr, int base);
unsigned long strtoul(const char *str, char **endptr, int base);

int sprintf(char *str, const char *format, ...);
int snprintf(char *str, size_t size, const char *format, ...);
int vsprintf(char *str, const char *format, va_list ap);
int vsnprintf(char *str, size_t size, const char *format, va_list ap);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int vprintf(const char *format, va_list ap);
int vfprintf(FILE *stream, const char *format, va_list ap);
int puts(const char *s);
int putchar(int c);
int getchar(void);
char *fgets(char *s, int size, FILE *stream);
char *gets(char *s);

FILE *fopen(const char *pathname, const char *mode);
int fclose(FILE *stream);
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream);
int fseek(FILE *stream, long offset, int whence);
long ftell(FILE *stream);
int fgetc(FILE *stream);
int fputc(int c, FILE *stream);
char *fgets(char *s, int size, FILE *stream);
int fputs(const char *s, FILE *stream);
int fflush(FILE *stream);
int feof(FILE *stream);
int ferror(FILE *stream);

int system(const char *command);

char *getenv(const char *name);
int setenv(const char *name, const char *value, int overwrite);
int unsetenv(const char *name);
int putenv(char *string);

#endif