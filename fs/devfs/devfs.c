#include <vfs.h>
#include <kernel.h>
#include <string.h>

#define DEVFS_MAX_DEVICES 32

struct devfs_node {
    char name[32];
    struct file_operations *fops;
    void *private_data;
    mode_t mode;
    struct devfs_node *next;
};

static struct devfs_node devfs_devices[DEVFS_MAX_DEVICES];
static uint32_t devfs_count = 0;

struct file_operations null_fops = {0};
struct file_operations zero_fops = {0};
struct file_operations random_fops = {0};
struct file_operations uart_fops = {0};

static ssize_t null_read(struct file *file, void *buf, size_t count) {
    (void)file; (void)buf; (void)count;
    return 0;
}

static ssize_t null_write(struct file *file, const void *buf, size_t count) {
    (void)file; (void)buf;
    return count;
}

static ssize_t zero_read(struct file *file, void *buf, size_t count) {
    (void)file;
    memset(buf, 0, count);
    return count;
}

static ssize_t zero_write(struct file *file, const void *buf, size_t count) {
    (void)file; (void)buf;
    return count;
}

static ssize_t random_read(struct file *file, void *buf, size_t count) {
    (void)file;
    uint32_t *dst = (uint32_t *)buf;
    for (size_t i = 0; i < count / 4; i++) {
        dst[i] = cpu_states[0].tick_count * 1664525 + 1013904223;
    }
    return count;
}

static ssize_t uart_read(struct file *file, void *buf, size_t count) {
    (void)file;
    extern int uart_getc(void);
    char *dst = (char *)buf;
    size_t read = 0;
    while (read < count) {
        int c = uart_getc();
        if (c < 0) break;
        *dst++ = c;
        read++;
        if (c == '\n') break;
    }
    return read;
}

static ssize_t uart_write(struct file *file, const void *buf, size_t count) {
    (void)file;
    extern void uart_puts(const char *);
    extern void uart_putc(char);
    const char *src = (const char *)buf;
    for (size_t i = 0; i < count; i++) {
        uart_putc(src[i]);
    }
    return count;
}

static int devfs_open(struct file *file, const char *path, int flags) {
    (void)flags;
    for (uint32_t i = 0; i < devfs_count; i++) {
        if (strcmp(devfs_devices[i].name, path + 1) == 0) {
            file->fops = devfs_devices[i].fops;
            file->private_data = devfs_devices[i].private_data;
            return 0;
        }
    }
    return -1;
}

static int devfs_readdir(struct file *file, struct dirent *ent) {
    (void)file;
    static uint32_t idx = 0;
    if (idx >= devfs_count) {
        idx = 0;
        return 0;
    }
    strncpy(ent->d_name, devfs_devices[idx].name, 255);
    ent->d_type = DT_CHR;
    idx++;
    return 1;
}

static struct file_operations devfs_fops = {
    .open = devfs_open,
    .readdir = devfs_readdir,
};

void devfs_register(const char *name, struct file_operations *fops, void *data, mode_t mode) {
    if (devfs_count >= DEVFS_MAX_DEVICES) return;
    strncpy(devfs_devices[devfs_count].name, name, 31);
    devfs_devices[devfs_count].fops = fops;
    devfs_devices[devfs_count].private_data = data;
    devfs_devices[devfs_count].mode = mode;
    devfs_count++;
}

int devfs_mount(const char *source, const char *target, int flags, void *data) {
    (void)source; (void)flags; (void)data;
    null_fops.read = null_read;
    null_fops.write = null_write;
    zero_fops.read = zero_read;
    zero_fops.write = zero_write;
    random_fops.read = random_read;
    uart_fops.read = uart_read;
    uart_fops.write = uart_write;

    devfs_register("null", &null_fops, NULL, S_IFCHR | 0666);
    devfs_register("zero", &zero_fops, NULL, S_IFCHR | 0666);
    devfs_register("random", &random_fops, NULL, S_IFCHR | 0444);
    devfs_register("uart0", &uart_fops, NULL, S_IFCHR | 0666);
    devfs_register("console", &uart_fops, NULL, S_IFCHR | 0666);

    return vfs_register_fs(target, &devfs_fops, NULL);
}