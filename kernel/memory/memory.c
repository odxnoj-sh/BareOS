#include <kernel.h>
#include <xtensa.h>
#include <esp32s3_reg.h>
#include <string.h>

#define HEAP_MAGIC 0xDEADBEEF
#define HEAP_MIN_BLOCK 16

struct heap_block {
    uint32_t magic;
    size_t size;
    uint8_t free;
    struct heap_block *next;
    struct heap_block *prev;
};

static struct heap_block *heap_start = NULL;
static struct heap_block *heap_end = NULL;
static uint8_t heap_initialized = 0;

void memory_init(void) {
    __asm__ volatile(
        "movi %0, _heap_start\n"
        "movi %1, _heap_end\n"
        : "=r"(heap_start), "=r"(heap_end)
        :
        :
    );

    size_t heap_size = (uint32_t)heap_end - (uint32_t)heap_start;
    heap_start->magic = HEAP_MAGIC;
    heap_start->size = heap_size - sizeof(struct heap_block);
    heap_start->free = 1;
    heap_start->next = NULL;
    heap_start->prev = NULL;

    heap_initialized = 1;
}

void *kmalloc(size_t size) {
    if (!heap_initialized) return NULL;
    if (size == 0) return NULL;

    size = (size + 7) & ~7;

    struct heap_block *block = heap_start;
    while (block) {
        if (block->free && block->size >= size) {
            if (block->size >= size + sizeof(struct heap_block) + HEAP_MIN_BLOCK) {
                struct heap_block *new_block = (struct heap_block *)((uint8_t *)block + sizeof(struct heap_block) + size);
                new_block->magic = HEAP_MAGIC;
                new_block->size = block->size - size - sizeof(struct heap_block);
                new_block->free = 1;
                new_block->next = block->next;
                new_block->prev = block;
                if (block->next) block->next->prev = new_block;
                block->next = new_block;
            }
            block->size = size;
            block->free = 0;
            return (uint8_t *)block + sizeof(struct heap_block);
        }
        block = block->next;
    }
    return NULL;
}

void *kzalloc(size_t size) {
    void *ptr = kmalloc(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void kfree(void *ptr) {
    if (!ptr) return;

    struct heap_block *block = (struct heap_block *)((uint8_t *)ptr - sizeof(struct heap_block));
    if (block->magic != HEAP_MAGIC) return;

    block->free = 1;

    if (block->next && block->next->free) {
        block->size += sizeof(struct heap_block) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }

    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(struct heap_block) + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
    }
}

void *kmalloc_aligned(size_t size, size_t align) {
    if (!heap_initialized) return NULL;
    if (size == 0) return NULL;

    size = (size + align - 1) & ~(align - 1);
    return kmalloc(size);
}

void *psram_malloc(size_t size) {
    return kmalloc(size);
}

void psram_free(void *ptr) {
    kfree(ptr);
}

void memory_stats(uint32_t *total, uint32_t *used, uint32_t *free) {
    *total = *used = *free = 0;
    struct heap_block *block = heap_start;
    while (block) {
        *total += block->size + sizeof(struct heap_block);
        if (block->free) {
            *free += block->size;
        } else {
            *used += block->size;
        }
        block = block->next;
    }
}