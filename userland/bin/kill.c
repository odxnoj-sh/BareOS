#include <libc.h>
#include <signal.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("usage: kill PID...\n");
        return 1;
    }
    int sig = SIGTERM;
    int start = 1;
    if (argv[1][0] == '-') {
        sig = atoi(argv[1] + 1);
        start = 2;
    }
    for (int i = start; i < argc; i++) {
        pid_t pid = atoi(argv[i]);
        if (kill(pid, sig) < 0) {
            printf("kill: failed to send signal to %d\n", pid);
        }
    }
    return 0;
}