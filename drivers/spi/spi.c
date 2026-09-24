#include <esp32s3_reg.h>
#include <stddef.h>

void spi_init(int host) {
    (void)host;
}

int spi_transfer(int host, const void *tx, void *rx, size_t len) {
    (void)host; (void)tx; (void)rx; (void)len;
    return 0;
}

void spi_set_frequency(int host, int freq) {
    (void)host; (void)freq;
}