#ifndef EXEC_H
#define EXEC_H

#define EXEC_MAGIC 0x42415245

struct exec_header {
    uint32_t magic;
    uint32_t entry;
    uint32_t text_size;
    uint32_t data_size;
    uint32_t bss_size;
    uint32_t stack_size;
};

int load_binary(const char *path, struct task *task, char **argv, char **envp);
int do_execve(const char *path, char **argv, char **envp);

#endif