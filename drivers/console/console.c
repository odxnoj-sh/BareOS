#include <esp32s3_reg.h>

void console_init(void) {
}

void console_putc(char c) {
    extern void uart_putc(char);
    uart_putc(c);
}

void console_puts(const char *s) {
    extern void uart_puts(const char *);
    uart_puts(s);
}

int console_getc(void) {
    extern int uart_getc(void);
    return uart_getc();
}