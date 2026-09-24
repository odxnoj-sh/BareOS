#include <kernel.h>

#define SIGINT 2
#define SIGTERM 15
#define SIGKILL 9
#define SIGCHLD 17

#define SIG_DFL ((sighandler_t)0)
#define SIG_IGN ((sighandler_t)1)
#define SIG_ERR ((sighandler_t)-1)

typedef void (*sighandler_t)(int);

struct sigaction {
    sighandler_t sa_handler;
    int sa_flags;
};

static struct sigaction signal_handlers[32];

extern int getpid(void);
extern void syscall_exit(int);

void signal_init(void) {
    for (int i = 0; i < 32; i++) {
        signal_handlers[i].sa_handler = SIG_DFL;
        signal_handlers[i].sa_flags = 0;
    }
}

sighandler_t signal(int signum, sighandler_t handler) {
    if (signum <= 0 || signum >= 32) return SIG_ERR;
    sighandler_t old = signal_handlers[signum].sa_handler;
    signal_handlers[signum].sa_handler = handler;
    return old;
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    if (signum <= 0 || signum >= 32) return -1;
    if (oldact) *oldact = signal_handlers[signum];
    if (act) signal_handlers[signum] = *act;
    return 0;
}

int kill(pid_t pid, int sig) {
    (void)pid;
    if (sig <= 0 || sig >= 32) return -1;
    return 0;
}

int raise(int sig) {
    return kill(getpid(), sig);
}

void signal_deliver(int sig) {
    if (sig <= 0 || sig >= 32) return;
    if (signal_handlers[sig].sa_handler == SIG_DFL) {
        if (sig == SIGKILL || sig == SIGTERM) {
            syscall_exit(128 + sig);
        }
    } else if (signal_handlers[sig].sa_handler != SIG_IGN) {
        signal_handlers[sig].sa_handler(sig);
    }
}