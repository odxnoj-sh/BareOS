#include <kernel.h>
#include <libc.h>
#include <vfs.h>

#define EXEC_MAGIC 0x42415245

struct exec_header {
    uint32_t magic;
    uint32_t entry;
    uint32_t text_size;
    uint32_t data_size;
    uint32_t bss_size;
    uint32_t stack_size;
};

int load_binary(const char *path, struct task *task, char *const argv[], char *const envp[]) {
    (void)argv; (void)envp;
    struct file *file = vfs_open(path, O_RDONLY);
    if (!file) {
        return -1;
    }

    struct exec_header hdr;
    ssize_t r = vfs_read(file, &hdr, sizeof(hdr));
    if (r != sizeof(hdr)) {
        vfs_close(file);
        return -1;
    }

    if (hdr.magic != EXEC_MAGIC) {
        vfs_close(file);
        return -1;
    }

    size_t total_size = hdr.text_size + hdr.data_size + hdr.bss_size;
    void *mem = kmalloc(total_size);
    if (!mem) {
        vfs_close(file);
        return -1;
    }

    r = vfs_read(file, mem, hdr.text_size + hdr.data_size);
    if (r != (ssize_t)(hdr.text_size + hdr.data_size)) {
        kfree(mem);
        vfs_close(file);
        return -1;
    }

    if (hdr.bss_size > 0) {
        memset((uint8_t *)mem + hdr.text_size + hdr.data_size, 0, hdr.bss_size);
    }

    vfs_close(file);

    task->context->pc = (uint32_t)mem + hdr.entry;
    task->context->a1 = (uint32_t)mem + hdr.text_size + hdr.data_size + hdr.bss_size;

    return 0;
}

int do_execve(const char *path, char *const argv[], char *const envp[]) {
    struct task *current = cpu_states[0].current_task;
    if (!current || !current->process) {
        return -1;
    }

    return load_binary(path, current, argv, envp);
}