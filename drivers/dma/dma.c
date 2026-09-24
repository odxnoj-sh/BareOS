#include <esp32s3_reg.h>
#include <stddef.h>

void dma_init(void) {
}

void *dma_alloc(size_t size) {
    (void)size;
    return NULL;
}

void dma_free(void *ptr) {
    (void)ptr;
}

int dma_transfer(void *src, void *dst, size_t len) {
    (void)src; (void)dst; (void)len;
    return 0;
}