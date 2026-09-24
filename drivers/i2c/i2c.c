#include <esp32s3_reg.h>
#include <stdint.h>
#include <stddef.h>

void i2c_init(int port) {
    (void)port;
}

int i2c_write(int port, uint8_t addr, const void *data, size_t len) {
    (void)port; (void)addr; (void)data; (void)len;
    return 0;
}

int i2c_read(int port, uint8_t addr, void *data, size_t len) {
    (void)port; (void)addr; (void)data; (void)len;
    return 0;
}

int i2c_write_read(int port, uint8_t addr, const void *tx, size_t tx_len, void *rx, size_t rx_len) {
    (void)port; (void)addr; (void)tx; (void)tx_len; (void)rx; (void)rx_len;
    return 0;
}