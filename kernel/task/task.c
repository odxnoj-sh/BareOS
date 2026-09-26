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
    task->heap_allocated = 0;
    task->heap_freed = 0;

    task->stack_size = TASK_STACK_SIZE;
    task->stack_base = kmalloc_aligned(TASK_STACK_SIZE, 16);
    if (!task->stack_base) {
        kfree(task);
        return NULL;
    }

    memset(task->stack_base, 0xA5, task->stack_size);
    task->stack_used = 0;

    task->context = (struct task_context *)((uint32_t)task->stack_base + task->stack_size - sizeof(struct task_context));
    memset(task->context, 0, sizeof(struct task_context));
    task->context->pc = (uint32_t)cpu_idle;
    task->context->ps = 0x00040000;
    task->context->a1 = (uint32_t)task->context;

    return task;
}

struct task *task_create_user(void (*entry)(void), uint8_t priority, size_t stack_size) {
    struct task *task = kzalloc(sizeof(struct task));
    if (!task) return NULL;

    task->tid = alloc_tid();
    if (!task->tid) {
        kfree(task);
        return NULL;
    }

    task->pid = 0;
    task->state = TASK_STATE_STOPPED;
    task->priority = priority;
    task->cpu_affinity = 0;
    task->flags = 0;
    task->process = NULL;
    task->exit_code = 0;
    task->heap_allocated = 0;
    task->heap_freed = 0;

    if (stack_size == 0) stack_size = TASK_STACK_SIZE;
    task->stack_size = stack_size;
    task->stack_base = kmalloc_aligned(stack_size, 16);
    if (!task->stack_base) {
        free_tid(task->tid);
        kfree(task);
        return NULL;
    }

    memset(task->stack_base, 0xA5, task->stack_size);
    task->stack_used = 0;

    task->context = (struct task_context *)((uint32_t)task->stack_base + task->stack_size - sizeof(struct task_context));
    memset(task->context, 0, sizeof(struct task_context));
    task->context->pc = (uint32_t)entry;
    task->context->ps = 0x00040000;
    task->context->a1 = (uint32_t)task->context;

    return task;
}

void task_destroy(struct task *task) {
    if (!task) return;

    if (task->stack_base) {
        kfree(task->stack_base);
        task->stack_base = NULL;
    }

    free_tid(task->tid);
    kfree(task);
}

void task_stack_watermark(struct task *task) {
    if (!task || !task->stack_base) return;

    uint8_t *stack = (uint8_t *)task->stack_base;
    size_t used = 0;

    for (size_t i = 0; i < task->stack_size; i++) {
        if (stack[i] != 0xA5) {
            used = task->stack_size - i;
            break;
        }
    }

    if (used > task->stack_used) {
        task->stack_used = used;
    }
}