#include <libc.h>
#include <wifi.h>

#define MAX_ARGS 32
#define MAX_LINE 512
#define MAX_HISTORY 8

static char history[MAX_HISTORY][MAX_LINE];
static int history_count = 0;
static int history_pos = 0;

int parse_line(char *line, char **argv, int max_args);
void execute_command(int argc, char **argv);
void builtin_cd(int argc, char **argv);
void builtin_echo(int argc, char **argv);
void builtin_env(void);
void builtin_export(int argc, char **argv);
void builtin_unset(int argc, char **argv);
void builtin_history(void);
void builtin_memstat(void);
void builtin_netstat(void);
void builtin_ping(int argc, char **argv);
void builtin_udptest(int argc, char **argv);
void builtin_ifconfig(int argc, char **argv);
void builtin_wifi(int argc, char **argv);

void shell_init(void) {
    extern char *cwd;
    cwd[0] = '/'; cwd[1] = 0;
}

void shell_main(void) {
    char line[MAX_LINE];
    shell_init();

    while (1) {
        printf("bare@esp32:%s$ ", cwd);
        fflush(stdout);

        if (!fgets(line, MAX_LINE, stdin)) continue;
        line[strcspn(line, "\n")] = 0;

        if (!*line) continue;

        if (history_count < MAX_HISTORY) {
            strcpy(history[history_count++], line);
        } else {
            for (int i = 0; i < MAX_HISTORY - 1; i++) {
                strcpy(history[i], history[i + 1]);
            }
            strcpy(history[MAX_HISTORY - 1], line);
        }
        history_pos = history_count;

        char *argv[MAX_ARGS];
        int argc = parse_line(line, argv, MAX_ARGS);
        if (argc == 0) continue;

        if (strcmp(argv[0], "exit") == 0) {
            exit(0);
        } else if (strcmp(argv[0], "cd") == 0) {
            builtin_cd(argc, argv);
        } else if (strcmp(argv[0], "pwd") == 0) {
            printf("%s\n", cwd);
        } else if (strcmp(argv[0], "echo") == 0) {
            builtin_echo(argc, argv);
        } else if (strcmp(argv[0], "env") == 0) {
            builtin_env();
        } else if (strcmp(argv[0], "export") == 0) {
            builtin_export(argc, argv);
        } else if (strcmp(argv[0], "unset") == 0) {
            builtin_unset(argc, argv);
        } else if (strcmp(argv[0], "history") == 0) {
            builtin_history();
        } else if (strcmp(argv[0], "memstat") == 0) {
            builtin_memstat();
        } else if (strcmp(argv[0], "netstat") == 0) {
            builtin_netstat();
        } else if (strcmp(argv[0], "ping") == 0) {
            builtin_ping(argc, argv);
        } else if (strcmp(argv[0], "udptest") == 0) {
            builtin_udptest(argc, argv);
        } else if (strcmp(argv[0], "ifconfig") == 0) {
            builtin_ifconfig(argc, argv);
        } else if (strcmp(argv[0], "wifi") == 0) {
            builtin_wifi(argc, argv);
        } else {
            execute_command(argc, argv);
        }
    }
}

int parse_line(char *line, char **argv, int max_args) {
    int argc = 0;
    char *p = line;

    while (*p && argc < max_args - 1) {
        while (*p && isspace(*p)) p++;
        if (!*p) break;

        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            argv[argc++] = p;
            while (*p && *p != quote) p++;
            if (*p) *p++ = 0;
        } else {
            argv[argc++] = p;
            while (*p && !isspace(*p)) p++;
            if (*p) *p++ = 0;
        }
    }
    argv[argc] = NULL;
    return argc;
}

void execute_command(int argc, char **argv) {
    (void)argc;
    pid_t pid = fork();
    if (pid == 0) {
        execvp(argv[0], argv);
        printf("bare: %s: command not found\n", argv[0]);
        _exit(127);
    } else if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
    }
}

void builtin_cd(int argc, char **argv) {
    const char *path = argc > 1 ? argv[1] : getenv("HOME");
    if (!path) path = "/";
    if (chdir(path) < 0) {
        printf("cd: %s: No such directory\n", path);
    }
}

void builtin_echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        printf("%s%s", argv[i], i + 1 < argc ? " " : "");
    }
    printf("\n");
}

void builtin_env(void) {
    extern char **environ;
    for (char **env = environ; *env; env++) {
        printf("%s\n", *env);
    }
}

void builtin_export(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        putenv(argv[i]);
    }
}

void builtin_unset(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        unsetenv(argv[i]);
    }
}

void builtin_history(void) {
    for (int i = 0; i < history_count; i++) {
        printf("%d  %s\n", i + 1, history[i]);
    }
}

void builtin_memstat(void) {
    uint32_t total, used, free;
    memory_stats(&total, &used, &free);
    printf("=== Memory Statistics ===\n");
    printf("Total:   %u KB\n", total / 1024);
    printf("Used:    %u KB\n", used / 1024);
    printf("Free:    %u KB\n", free / 1024);
    printf("\n");

    for (int i = 0; i < 2; i++) {
        uint32_t r_total, r_used, r_free, r_blocks;
        memory_region_stats(i, &r_total, &r_used, &r_free, &r_blocks);
        if (r_total == 0) continue;
        printf("Region %d:\n", i);
        printf("  Total:  %u KB\n", r_total / 1024);
        printf("  Used:   %u KB\n", r_used / 1024);
        printf("  Free:   %u KB\n", r_free / 1024);
        printf("  Blocks: %u\n", r_blocks);
    }

    if (psram_is_available()) {
        printf("\nPSRAM:\n");
        printf("  Total: %u KB\n", psram_get_total() / 1024);
        printf("  Used:  %u KB\n", psram_get_used() / 1024);
        printf("  Free:  %u KB\n", psram_get_free() / 1024);
    }

    printf("\nInternal DRAM:\n");
    printf("  Total: %u KB\n", internal_get_total() / 1024);
    printf("  Used:  %u KB\n", internal_get_used() / 1024);
    printf("  Free:  %u KB\n", internal_get_free() / 1024);

    int check = memory_check_heap();
    printf("\nHeap integrity: %s\n", check == 0 ? "OK" : "CORRUPTED");
}

void builtin_netstat(void) {
    extern void netstat_print(void);
    netstat_print();
}

void builtin_ping(int argc, char **argv) {
    (void)argc; (void)argv;
    printf("Ping not yet implemented\n");
}

void builtin_udptest(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: udptest server|client [port]\n");
        return;
    }

    int port = argc > 2 ? atoi(argv[2]) : 4000;

    if (strcmp(argv[1], "server") == 0) {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            printf("socket failed\n");
            return;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr = 0x7F000001;
        addr.sin_port = port;

        if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
            printf("bind failed\n");
            close(sock);
            return;
        }

        printf("UDP server listening on 127.0.0.1:%d\n", port);

        char buf[256];
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(from);

        while (1) {
            int n = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from, &fromlen);
            if (n > 0) {
                buf[n] = 0;
                printf("Received: %s\n", buf);
            }
        }
    } else if (strcmp(argv[1], "client") == 0) {
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock < 0) {
            printf("socket failed\n");
            return;
        }

        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr = 0x7F000001;
        addr.sin_port = port;

        char *msg = "Hello UDP!";
        int n = sendto(sock, msg, strlen(msg), 0, (struct sockaddr *)&addr, sizeof(addr));
        if (n > 0) {
            printf("Sent %d bytes to 127.0.0.1:%d\n", n, port);
        } else {
            printf("sendto failed\n");
        }

        close(sock);
    } else {
        printf("Usage: udptest server|client [port]\n");
    }
}

void builtin_ifconfig(int argc, char **argv) {
    (void)argc; (void)argv;
    extern void netstat_print(void);

    netstat_print();

    int wstate = wifi_get_state();
    const char *state_str = "DOWN";
    switch (wstate) {
        case 0: state_str = "DOWN"; break;
        case 1: state_str = "INIT"; break;
        case 2: state_str = "SCANNING"; break;
        case 3: state_str = "AUTHENTICATING"; break;
        case 4: state_str = "ASSOCIATING"; break;
        case 5: state_str = "ASSOCIATED"; break;
        case 6: state_str = "FAILED"; break;
    }

    printf("\nwlan0:\n");
    printf("  state: %s\n", state_str);
    const uint8_t *mac = wifi_get_mac();
    printf("  MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", 
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    if (wstate == 5) {
        const char *ssid = wifi_get_ssid();
        if (ssid && *ssid) {
            printf("  SSID: %s\n", ssid);
        }
        int rssi = wifi_get_rssi();
        if (rssi != 0) {
            printf("  RSSI: %d dBm\n", rssi);
        }
        uint8_t channel = wifi_get_channel();
        if (channel != 0) {
            printf("  Channel: %d\n", channel);
        }
    }
}

void builtin_wifi(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: wifi connect <ssid> [password]\n");
        printf("       wifi disconnect\n");
        printf("       wifi status\n");
        return;
    }

    if (strcmp(argv[1], "connect") == 0) {
        if (argc < 3) {
            printf("Usage: wifi connect <ssid> [password]\n");
            return;
        }
        const char *ssid = argv[2];
        const char *pass = argc > 3 ? argv[3] : "";
        printf("Connecting to %s...\n", ssid);
        wifi_set_credentials(ssid, pass, 0);
        int ret = wifi_assoc(ssid, pass);
        if (ret == 0) {
            printf("Association started\n");
        } else if (ret == -2) {
            printf("Security mode not supported (only open networks)\n");
        } else {
            printf("Association failed: %d\n", ret);
        }
    } else if (strcmp(argv[1], "disconnect") == 0) {
        wifi_stop();
        printf("Disconnected\n");
    } else if (strcmp(argv[1], "status") == 0) {
        int wstate = wifi_get_state();
        const char *state_str = "DOWN";
        switch (wstate) {
            case 0: state_str = "DOWN"; break;
            case 1: state_str = "INIT"; break;
            case 2: state_str = "SCANNING"; break;
            case 3: state_str = "AUTHENTICATING"; break;
            case 4: state_str = "ASSOCIATING"; break;
            case 5: state_str = "ASSOCIATED"; break;
            case 6: state_str = "FAILED"; break;
        }
        printf("WiFi state: %s\n", state_str);
        if (wstate == 5) {
            const char *ssid = wifi_get_ssid();
            if (ssid && *ssid) {
                printf("SSID: %s\n", ssid);
            }
        }
    } else {
        printf("Unknown wifi command\n");
    }
}