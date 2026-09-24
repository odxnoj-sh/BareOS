#include <esp32s3_reg.h>
#include <xtensa.h>

#define REG_WRITE(addr, val) (*(volatile uint32_t *)(addr) = (val))
#define REG_READ(addr) (*(volatile uint32_t *)(addr))

#define UART_CLK_FREQ 80000000
#define UART_BAUD_RATE 115200

void uart_init(void) {
    uint32_t clkdiv = UART_CLK_FREQ / UART_BAUD_RATE;
    REG_WRITE(UART_CLKDIV_REG(DR_REG_UART_BASE), clkdiv);

    REG_WRITE(UART_CONF0_REG(DR_REG_UART_BASE), 0);
    REG_WRITE(UART_CONF1_REG(DR_REG_UART_BASE), 0);

    REG_WRITE(UART_INT_ENA_REG(DR_REG_UART_BASE), 0);
    REG_WRITE(UART_INT_CLR_REG(DR_REG_UART_BASE), 0xFFFFFFFF);
}

void uart_putc(char c) {
    while (REG_READ(UART_STATUS_REG(DR_REG_UART_BASE)) & (1 << 1)) {
    }
    REG_WRITE(UART_FIFO_REG(DR_REG_UART_BASE), c);
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r');
        uart_putc(*s++);
    }
}

void uart_puthex(uint32_t val) {
    const char *hex = "0123456789abcdef";
    for (int i = 28; i >= 0; i -= 4) {
        uart_putc(hex[(val >> i) & 0xF]);
    }
}

void uart_putdec(uint32_t val) {
    char buf[12];
    int i = 0;
    if (val == 0) {
        uart_putc('0');
        return;
    }
    while (val > 0) {
        buf[i++] = '0' + (val % 10);
        val /= 10;
    }
    while (i--) {
        uart_putc(buf[i]);
    }
}

int uart_getc(void) {
    if (REG_READ(UART_STATUS_REG(DR_REG_UART_BASE)) & (1 << 0)) {
        return REG_READ(UART_FIFO_REG(DR_REG_UART_BASE)) & 0xFF;
    }
    return -1;
}