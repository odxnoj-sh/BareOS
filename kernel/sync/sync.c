#include <kernel.h>
#include <xtensa.h>

#define SPINLOCK_UNLOCKED 0
#define SPINLOCK_LOCKED 1

void spinlock_init(volatile int *lock) {
    *lock = SPINLOCK_UNLOCKED;
}

void spinlock_acquire(volatile int *lock) {
    while (__sync_lock_test_and_set(lock, SPINLOCK_LOCKED)) {
        while (*lock == SPINLOCK_LOCKED) {
        }
    }
}

void spinlock_release(volatile int *lock) {
    __sync_lock_release(lock);
}

struct mutex {
    volatile int locked;
    struct task *owner;
    int count;
};

void mutex_init(struct mutex *m) {
    m->locked = 0;
    m->owner = NULL;
    m->count = 0;
}

void mutex_lock(struct mutex *m) {
    struct task *current = cpu_states[0].current_task;
    if (!current) return;

    uint32_t psr = read_psr();
    write_psr(psr | 0xF);

    if (m->locked && m->owner == current) {
        m->count++;
        write_psr(psr);
        return;
    }

    while (__sync_lock_test_and_set(&m->locked, 1)) {
        write_psr(psr);
        task_yield();
        psr = read_psr();
        write_psr(psr | 0xF);
    }

    m->owner = current;
    m->count = 1;
    write_psr(psr);
}

void mutex_unlock(struct mutex *m) {
    struct task *current = cpu_states[0].current_task;
    if (!current || m->owner != current) return;

    if (--m->count > 0) return;
    m->owner = NULL;
    __sync_lock_release(&m->locked);
}

struct semaphore {
    volatile int count;
    struct task *waiters;
};

void semaphore_init(struct semaphore *s, int value) {
    s->count = value;
    s->waiters = NULL;
}

void semaphore_wait(struct semaphore *s) {
    struct task *current = cpu_states[0].current_task;
    if (!current) return;

    uint32_t psr = read_psr();
    write_psr(psr | 0xF);

    if (s->count > 0) {
        s->count--;
        write_psr(psr);
        return;
    }

    current->next = s->waiters;
    s->waiters = current;
    current->state = TASK_STATE_BLOCKED;
    write_psr(psr);
    schedule();
}

void semaphore_signal(struct semaphore *s) {
    uint32_t psr = read_psr();
    write_psr(psr | 0xF);

    if (s->waiters) {
        struct task *t = s->waiters;
        s->waiters = t->next;
        task_wake(t);
    } else {
        s->count++;
    }
    write_psr(psr);
}