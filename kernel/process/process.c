#include <kernel.h>
#include <xtensa.h>
#include <memory.h>
#include <string.h>

extern void vfs_close(struct file *);
#define file_close(f) vfs_close(f)

struct task *task_create(struct process *proc, void (*entry)(void), uint8_t priority) {
    if (!proc) return NULL;

    struct task *task = kzalloc(sizeof(struct task));
    if (!task) return NULL;

    task->tid = alloc_tid();
    if (!task->tid) {
        kfree(task);
        return NULL;
    }

    task->pid = proc->pid;
    task->state = TASK_STATE_STOPPED;
    task->priority = priority;
    task->cpu_affinity = 0;
    task->flags = 0;
    task->process = proc;
    task->exit_code = 0;

    task->stack_size = TASK_STACK_SIZE;
    task->stack_base = kmalloc_aligned(TASK_STACK_SIZE, 16);
    if (!task->stack_base) {
        free_tid(task->tid);
        kfree(task);
        return NULL;
    }

    task->context = (struct task_context *)((uint32_t)task->stack_base + task->stack_size - sizeof(struct task_context));
    memset(task->context, 0, sizeof(struct task_context));
    task->context->pc = (uint32_t)entry;
    task->context->ps = 0x00040000;
    task->context->a1 = (uint32_t)task->context;

    task->next = proc->tasks;
    task->prev = NULL;
    if (proc->tasks) proc->tasks->prev = task;
    proc->tasks = task;
    proc->task_count++;

    task_table[task->tid] = task;
    return task;
}

void task_destroy(struct task *task) {
    if (!task) return;

    if (task->stack_base) {
        kfree(task->stack_base);
    }

    if (task->prev) task->prev->next = task->next;
    if (task->next) task->next->prev = task->prev;
    if (task->process && task->process->tasks == task) {
        task->process->tasks = task->next;
    }
    if (task->process) task->process->task_count--;

    free_tid(task->tid);
    kfree(task);
}

struct process *process_create(const char *name) {
    struct process *proc = kzalloc(sizeof(struct process));
    if (!proc) return NULL;

    proc->pid = alloc_pid();
    if (!proc->pid) {
        kfree(proc);
        return NULL;
    }

    proc->ppid = 0;
    proc->uid = 0;
    proc->gid = 0;
    if (name) {
        strncpy(proc->name, name, 31);
        proc->name[31] = 0;
    }

    proc->fd_max = 32;
    proc->fd_table = kzalloc(sizeof(struct file *) * proc->fd_max);
    if (!proc->fd_table) {
        free_pid(proc->pid);
        kfree(proc);
        return NULL;
    }
    proc->fd_count = 0;

    proc->heap_start = NULL;
    proc->heap_end = NULL;
    proc->heap_brk = NULL;

    process_table[proc->pid] = proc;
    return proc;
}

void process_destroy(struct process *proc) {
    if (!proc) return;

    struct task *task = proc->tasks;
    while (task) {
        struct task *next = task->next;
        task_destroy(task);
        task = next;
    }

    if (proc->fd_table) {
        for (int i = 0; i < proc->fd_count; i++) {
            if (proc->fd_table[i]) {
                file_close(proc->fd_table[i]);
            }
        }
        kfree(proc->fd_table);
    }

    free_pid(proc->pid);
    kfree(proc);
}

struct file **get_fd_table(void) {
    struct task *current = cpu_states[0].current_task;
    if (!current || !current->process) return NULL;
    return current->process->fd_table;
}

int get_fd_count(void) {
    struct task *current = cpu_states[0].current_task;
    if (!current || !current->process) return 0;
    return current->process->fd_count;
}