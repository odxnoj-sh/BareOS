#include <libc.h>

int main(int argc, char **argv) {
    int show_all = 0, show_sysname = 0, show_nodename = 0, show_release = 0, show_version = 0, show_machine = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-a") == 0) show_all = 1;
        else if (strcmp(argv[i], "-s") == 0) show_sysname = 1;
        else if (strcmp(argv[i], "-n") == 0) show_nodename = 1;
        else if (strcmp(argv[i], "-r") == 0) show_release = 1;
        else if (strcmp(argv[i], "-v") == 0) show_version = 1;
        else if (strcmp(argv[i], "-m") == 0) show_machine = 1;
    }

    if (!show_all && !show_sysname && !show_nodename && !show_release && !show_version && !show_machine) {
        show_sysname = 1;
    }

    if (show_all || show_sysname) printf("BareOS");
    if (show_all || show_nodename) printf(" esp32");
    if (show_all || show_release) printf(" 0.1.0");
    if (show_all || show_version) printf(" #1");
    if (show_all || show_machine) printf(" xtensa");
    printf("\n");
    return 0;
}