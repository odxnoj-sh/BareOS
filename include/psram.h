#ifndef PSRAM_H
#define PSRAM_H

#include <stdint.h>
#include <stddef.h>

#define PSRAM_BASE 0x3F800000

int psram_init(void);
int psram_read(uint32_t addr, void *buf, size_t len);
int psram_write(uint32_t addr, const void *buf, size_t len);
uint32_t psram_get_size(void);

#endif