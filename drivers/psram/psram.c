#include <esp32s3_reg.h>
#include <stdint.h>
#include <stddef.h>

void psram_init(void) {
}

int psram_read(uint32_t addr, void *buf, size_t len) {
    (void)addr; (void)buf; (void)len;
    return 0;
}

int psram_write(uint32_t addr, const void *buf, size_t len) {
    (void)addr; (void)buf; (void)len;
    return 0;
}

uint32_t psram_get_size(void) {
    return 8 * 1024 * 1024;
}