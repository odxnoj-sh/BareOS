#include <libc.h>
#include <kernel.h>

extern struct task *task_table[];
extern struct process *process_table[];

int main(int argc, char **argv) {
    printf("PID  TID  STATE     PRI CPU COMMAND\n");
    for (int i = 1; i < MAX_TASKS; i++) {
        struct task *t = task_table[i];
        if (!t) continue;
        const char *state_str[] = {"RUNNABLE", "RUNNING", "SLEEPING", "BLOCKED", "STOPPED", "EXITED"};
        printf("%-4d %-4d %-9s %-3d %-3d %s\n",
            t->pid, t->tid, state_str[t->state], t->priority, t->cpu_affinity, t->process ? t->process->name : "?");
    }
    return 0;
}