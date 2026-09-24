#include <kernel.h>

int storage_init(void) {
    return 0;
}

int storage_read(uint32_t block, void *buf, uint32_t count) {
    (void)block; (void)buf; (void)count;
    return 0;
}

int storage_write(uint32_t block, const void *buf, uint32_t count) {
    (void)block; (void)buf; (void)count;
    return 0;
}

uint32_t storage_get_size(void) {
    return 0;
}