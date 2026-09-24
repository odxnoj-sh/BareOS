#include <libc.h>
#include <kernel.h>

static unsigned int rand_seed = 1;

void *malloc(size_t size) {
    return kmalloc(size);
}

void free(void *ptr) {
    kfree(ptr);
}

void *calloc(size_t nmemb, size_t size) {
    size_t total = nmemb * size;
    void *ptr = kmalloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void *realloc(void *ptr, size_t size) {
    (void)ptr;
    (void)size;
    return kmalloc(size);
}

int atoi(const char *str) {
    int val = 0, neg = 0;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == '-') { neg = 1; str++; }
    else if (*str == '+') str++;
    while (*str >= '0' && *str <= '9') {
        val = val * 10 + (*str - '0');
        str++;
    }
    return neg ? -val : val;
}

long atol(const char *str) {
    long val = 0;
    int neg = 0;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == '-') { neg = 1; str++; }
    else if (*str == '+') str++;
    while (*str >= '0' && *str <= '9') {
        val = val * 10 + (*str - '0');
        str++;
    }
    return neg ? -val : val;
}

long strtol(const char *str, char **endptr, int base) {
    long val = 0;
    int neg = 0;
    while (*str == ' ' || *str == '\t') str++;
    if (*str == '-') { neg = 1; str++; }
    else if (*str == '+') str++;
    if (base == 0) {
        if (*str == '0') {
            if (str[1] == 'x' || str[1] == 'X') { base = 16; str += 2; }
            else { base = 8; }
        } else base = 10;
    }
    while (*str) {
        int digit = 0;
        if (*str >= '0' && *str <= '9') digit = *str - '0';
        else if (*str >= 'a' && *str <= 'z') digit = *str - 'a' + 10;
        else if (*str >= 'A' && *str <= 'Z') digit = *str - 'A' + 10;
        else break;
        if (digit >= base) break;
        val = val * base + digit;
        str++;
    }
    if (endptr) *endptr = (char *)str;
    return neg ? -val : val;
}

unsigned long strtoul(const char *str, char **endptr, int base) {
    return (unsigned long)strtol(str, endptr, base);
}

int rand(void) {
    rand_seed = rand_seed * 1103515245 + 12345;
    return (rand_seed >> 16) & 0x7FFF;
}

void srand(unsigned int seed) {
    rand_seed = seed;
}

void exit(int status) {
    extern void syscall_exit(int);
    syscall_exit(status);
}

void _exit(int status) {
    exit(status);
}

int abs(int x) {
    return x < 0 ? -x : x;
}

long labs(long x) {
    return x < 0 ? -x : x;
}

int system(const char *command) {
    (void)command;
    return -1;
}

char *getenv(const char *name) {
    extern char **environ;
    if (!environ) return NULL;
    size_t len = strlen(name);
    for (char **env = environ; *env; env++) {
        if (strncmp(*env, name, len) == 0 && (*env)[len] == '=') {
            return *env + len + 1;
        }
    }
    return NULL;
}

int setenv(const char *name, const char *value, int overwrite) {
    (void)name; (void)value; (void)overwrite;
    return 0;
}

int unsetenv(const char *name) {
    (void)name;
    return 0;
}

char *getcwd(char *buf, size_t size) {
    extern char *cwd;
    if (!buf) buf = malloc(size);
    if (!buf) return NULL;
    strncpy(buf, cwd, size);
    return buf;
}

int chdir(const char *path) {
    extern char *cwd;
    strncpy(cwd, path, 255);
    return 0;
}

char **environ = NULL;

char *cwd = "/";