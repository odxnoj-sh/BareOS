#include <esp32s3_reg.h>
#include <stdint.h>
#include <stddef.h>

void flash_init(void) {
}

int flash_read(uint32_t addr, void *buf, size_t len) {
    (void)addr; (void)buf; (void)len;
    return 0;
}

int flash_write(uint32_t addr, const void *buf, size_t len) {
    (void)addr; (void)buf; (void)len;
    return 0;
}

int flash_erase(uint32_t addr, size_t len) {
    (void)addr; (void)len;
    return 0;
}

uint32_t flash_get_size(void) {
    return 16 * 1024 * 1024;
}