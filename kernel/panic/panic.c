#include <kernel.h>
#include <xtensa.h>
#include <esp32s3_reg.h>

extern void uart_puts(const char *);
extern void uart_puthex(uint32_t);
extern void uart_putdec(uint32_t);

void kernel_panic(const char *msg) {
    uart_puts("KERNEL PANIC: ");
    uart_puts(msg);
    uart_puts("\n");

    uart_puts("CPU state:\n");
    uart_puts("  PSR: "); uart_puthex(read_psr()); uart_puts("\n");
    uart_puts("  EPC: "); uart_puthex(read_epc()); uart_puts("\n");
    uart_puts("  EXCCAUSE: "); uart_puthex(read_exccause()); uart_puts("\n");
    uart_puts("  EXCVADDR: "); uart_puthex(read_excvaddr()); uart_puts("\n");

    struct task *current = cpu_states[0].current_task;
    if (current) {
        uart_puts("Current task: "); uart_puts(current->process ? current->process->name : "kernel"); uart_puts("\n");
        uart_puts("  PID: "); uart_putdec(current->pid); uart_puts("\n");
        uart_puts("  TID: "); uart_putdec(current->tid); uart_puts("\n");
    }

    while (1) {
        waiti(0);
    }
}