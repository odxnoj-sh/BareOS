#include <kernel.h>
#include <xtensa.h>
#include <memory.h>
#include <string.h>

extern void cpu_idle(void);
extern struct task *runnable_queue[32];
extern struct task *sleep_queue;

struct task *task_create_idle(void) {
    struct task *task = kzalloc(sizeof(struct task));
    if (!task) return NULL;

    task->tid = alloc_tid();
    task->pid = 0;
    task->state = TASK_STATE_RUNNABLE;
    task->priority = PRIORITY_MIN;
    task->cpu_affinity = 0;
    task->flags = 1;
    task->process = NULL;
    task->exit_code = 0;

    task->stack_size = TASK_STACK_SIZE;
    task->stack_base = kmalloc_aligned(TASK_STACK_SIZE, 16);
    if (!task->stack_base) {
        kfree(task);
        return NULL;
    }

    task->context = (struct task_context *)((uint32_t)task->stack_base + task->stack_size - sizeof(struct task_context));
    memset(task->context, 0, sizeof(struct task_context));
    task->context->pc = (uint32_t)cpu_idle;
    task->context->ps = 0x00040000;
    task->context->a1 = (uint32_t)task->context;

    return task;
}