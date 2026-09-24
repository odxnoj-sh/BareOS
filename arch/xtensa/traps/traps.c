#include <xtensa.h>
#include <kernel.h>
#include <esp32s3_reg.h>

extern void _vector_table(void);
extern void uart_puts(const char *);
extern void uart_puthex(uint32_t);
extern void kernel_panic(const char *);

static void (*exception_handlers[32])(struct task_context *);
static void (*interrupt_handlers[32])(void);

void trap_init(void) {
    for (int i = 0; i < 32; i++) {
        exception_handlers[i] = NULL;
        interrupt_handlers[i] = NULL;
    }

    extern void _default_exception_handler(struct task_context *);
    for (int i = 0; i < 32; i++) {
        exception_handlers[i] = _default_exception_handler;
    }

    extern void _timer_interrupt_handler(void);
    interrupt_handlers[6] = _timer_interrupt_handler;

    uint32_t vecbase = (uint32_t)_vector_table;
    __asm__ volatile("wsr.vecbase %0" :: "r"(vecbase));
    rsync();

    write_intenable(0);
    write_intclear(0xFFFFFFFF);
}

void trap_register_exception(int cause, void (*handler)(struct task_context *)) {
    if (cause >= 0 && cause < 32) {
        exception_handlers[cause] = handler;
    }
}

void trap_register_interrupt(int num, void (*handler)(void)) {
    if (num >= 0 && num < 32) {
        interrupt_handlers[num] = handler;
    }
}

void interrupt_enable(int num) {
    uint32_t intena = read_intenable();
    intena |= (1 << num);
    write_intenable(intena);
}

void interrupt_disable(int num) {
    uint32_t intena = read_intenable();
    intena &= ~(1 << num);
    write_intenable(intena);
}

void _default_exception_handler(struct task_context *ctx) {
    uart_puts("Exception: ");
    uart_puthex(read_exccause());
    uart_puts("\nEPC: ");
    uart_puthex(ctx->pc);
    uart_puts("\nPS: ");
    uart_puthex(ctx->ps);
    uart_puts("\n");

    kernel_panic("Unhandled exception");
}

void _timer_interrupt_handler(void) {
    write_ccompare(0, read_ccount() + 240000);
    scheduler_tick();
    schedule();
}

void trap_dispatch_exception(struct task_context *ctx) {
    uint32_t cause = read_exccause();
    if (cause == EXCCAUSE_SYSCALL) {
        syscall_handler(ctx);
    } else if (cause < 32 && exception_handlers[cause]) {
        exception_handlers[cause](ctx);
    } else {
        _default_exception_handler(ctx);
    }
}

void trap_dispatch_interrupt(void) {
    uint32_t intraw = read_intenable();
    for (int i = 0; i < 32; i++) {
        if (intraw & (1 << i)) {
            if (interrupt_handlers[i]) {
                interrupt_handlers[i]();
            }
            write_intclear(1 << i);
        }
    }
}