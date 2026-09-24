#include <kernel.h>
#include <xtensa.h>

extern void context_switch(struct task *prev, struct task *next);

static struct task *runnable_queue[32];
static struct task *sleep_queue;

void scheduler_init(void) {
    for (int i = 0; i < 32; i++) {
        runnable_queue[i] = NULL;
    }
    sleep_queue = NULL;
}

void task_enqueue(struct task *task) {
    int prio = task->priority;
    if (prio > PRIORITY_MAX) prio = PRIORITY_MAX;
    task->next = runnable_queue[prio];
    task->prev = NULL;
    if (runnable_queue[prio]) {
        runnable_queue[prio]->prev = task;
    }
    runnable_queue[prio] = task;
    task->state = TASK_STATE_RUNNABLE;
}

void task_dequeue(struct task *task) {
    int prio = task->priority;
    if (prio > PRIORITY_MAX) prio = PRIORITY_MAX;
    if (task->prev) {
        task->prev->next = task->next;
    } else {
        runnable_queue[prio] = task->next;
    }
    if (task->next) {
        task->next->prev = task->prev;
    }
    task->next = task->prev = NULL;
}

struct task *scheduler_pick_next(void) {
    for (int prio = PRIORITY_MAX; prio >= 0; prio--) {
        if (runnable_queue[prio]) {
            struct task *task = runnable_queue[prio];
            task_dequeue(task);
            return task;
        }
    }
    return NULL;
}

void task_sleep(uint32_t ticks) {
    struct task *current = cpu_states[0].current_task;
    if (!current) return;

    current->wake_time = cpu_states[0].tick_count + ticks;
    current->state = TASK_STATE_SLEEPING;

    current->next = sleep_queue;
    if (sleep_queue) sleep_queue->prev = current;
    sleep_queue = current;

    schedule();
}

void task_wake(struct task *task) {
    if (task->state == TASK_STATE_SLEEPING) {
        if (task->prev) task->prev->next = task->next;
        if (task->next) task->next->prev = task->prev;
        if (sleep_queue == task) sleep_queue = task->next;
        task_enqueue(task);
    } else if (task->state == TASK_STATE_BLOCKED) {
        task_enqueue(task);
    }
}

void scheduler_tick(void) {
    cpu_states[0].tick_count++;

    struct task *t = sleep_queue;
    while (t) {
        struct task *next = t->next;
        if (cpu_states[0].tick_count >= t->wake_time) {
            if (t->prev) t->prev->next = t->next;
            if (t->next) t->next->prev = t->prev;
            if (sleep_queue == t) sleep_queue = t->next;
            t->prev = t->next = NULL;
            task_enqueue(t);
        }
        t = next;
    }
}

void schedule(void) {
    struct cpu_state *cpu = &cpu_states[0];
    struct task *prev = cpu->current_task;
    struct task *next = scheduler_pick_next();

    if (!next) {
        next = cpu->idle_task;
    }

    if (next == prev) return;

    cpu->current_task = next;
    next->state = TASK_STATE_RUNNING;

    if (prev && prev->state == TASK_STATE_RUNNING) {
        prev->state = TASK_STATE_RUNNABLE;
        task_enqueue(prev);
    }

    context_switch(prev, next);
}

void task_yield(void) {
    schedule();
}