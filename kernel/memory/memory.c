#include <kernel.h>
#include <xtensa.h>
#include <esp32s3_reg.h>
#include <string.h>
#include <psram.h>

#define HEAP_MAGIC 0xDEADBEEF
#define HEAP_FREE_MAGIC 0xFEEBFEEB
#define HEAP_MIN_BLOCK 16
#define HEAP_ALIGN 8

#define MEM_REGION_INTERNAL 0
#define MEM_REGION_PSRAM 1

struct heap_block {
    uint32_t magic;
    size_t size;
    uint8_t free;
    uint8_t region;
    struct heap_block *next;
    struct heap_block *prev;
};

struct mem_region {
    uint32_t start;
    uint32_t end;
    uint32_t size;
    uint32_t used;
    uint32_t free;
    struct heap_block *free_list;
    struct heap_block *block_list;
    int initialized;
    char name[16];
};

static struct mem_region regions[2];
static int num_regions = 0;
static int psram_available = 0;

static struct mutex heap_lock;

static inline int region_for_ptr(void *ptr) {
    uint32_t addr = (uint32_t)ptr;
    for (int i = 0; i < num_regions; i++) {
        if (addr >= regions[i].start && addr < regions[i].end) {
            return i;
        }
    }
    return -1;
}

static inline void heap_lock_acquire(void) {
    mutex_lock(&heap_lock);
}

static inline void heap_lock_release(void) {
    mutex_unlock(&heap_lock);
}

static void region_init(struct mem_region *r, const char *name, uint32_t start, uint32_t end) {
    r->start = start;
    r->end = end;
    r->size = end - start;
    r->used = 0;
    r->free = 0;
    r->free_list = NULL;
    r->block_list = NULL;
    r->initialized = 1;
    strncpy(r->name, name, 15);
    r->name[15] = 0;
}

static void region_add_block(struct mem_region *r, struct heap_block *block) {
    block->next = r->block_list;
    if (r->block_list) {
        r->block_list->prev = block;
    }
    r->block_list = block;
}

static struct heap_block *region_find_free(struct mem_region *r, size_t size) {
    struct heap_block *best = NULL;
    struct heap_block *block = r->free_list;

    while (block) {
        if (block->free && block->size >= size) {
            if (!best || block->size < best->size) {
                best = block;
            }
        }
        block = block->next;
    }
    return best;
}

static void region_remove_free(struct mem_region *r, struct heap_block *block) {
    if (block->prev) block->prev->next = block->next;
    if (block->next) block->next->prev = block->prev;
    if (r->free_list == block) r->free_list = block->next;
    block->next = block->prev = NULL;
}

static void region_add_free(struct mem_region *r, struct heap_block *block) {
    block->next = r->free_list;
    block->prev = NULL;
    if (r->free_list) r->free_list->prev = block;
    r->free_list = block;
}

static void region_split_block(struct mem_region *r, struct heap_block *block, size_t size) {
    if (block->size < size + sizeof(struct heap_block) + HEAP_MIN_BLOCK) {
        return;
    }

    struct heap_block *new_block = (struct heap_block *)((uint8_t *)block + sizeof(struct heap_block) + size);
    new_block->magic = HEAP_MAGIC;
    new_block->size = block->size - size - sizeof(struct heap_block);
    new_block->free = 1;
    new_block->region = block->region;
    new_block->next = block->next;
    new_block->prev = block;

    if (block->next) block->next->prev = new_block;
    block->next = new_block;
    block->size = size;

    region_add_block(r, new_block);
    region_add_free(r, new_block);
    r->free += new_block->size + sizeof(struct heap_block);
}

static void region_coalesce(struct mem_region *r, struct heap_block *block) {
    if (block->next && block->next->free) {
        region_remove_free(r, block->next);
        block->size += sizeof(struct heap_block) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }
    if (block->prev && block->prev->free) {
        region_remove_free(r, block->prev);
        block->prev->size += sizeof(struct heap_block) + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
        block = block->prev;
    }
    region_add_free(r, block);
}

void memory_init(void) {
    mutex_init(&heap_lock);

    extern uint32_t _heap_start, _heap_end;

    region_init(&regions[MEM_REGION_INTERNAL], "internal", (uint32_t)&_heap_start, (uint32_t)&_heap_end);
    num_regions = 1;

    struct heap_block *block = (struct heap_block *)regions[0].start;
    size_t heap_size = regions[0].end - regions[0].start;
    block->magic = HEAP_MAGIC;
    block->size = heap_size - sizeof(struct heap_block);
    block->free = 1;
    block->region = MEM_REGION_INTERNAL;
    block->next = NULL;
    block->prev = NULL;

    region_add_block(&regions[0], block);
    region_add_free(&regions[0], block);
    regions[0].free = block->size;

    if (psram_init() == 0) {
        psram_available = 1;
        uint32_t psram_sz = psram_get_size();
        region_init(&regions[MEM_REGION_PSRAM], "psram", PSRAM_BASE, PSRAM_BASE + psram_sz);
        num_regions = 2;

        struct heap_block *pblock = (struct heap_block *)regions[1].start;
        size_t psram_heap_size = regions[1].size - sizeof(struct heap_block);
        pblock->magic = HEAP_MAGIC;
        pblock->size = psram_heap_size - sizeof(struct heap_block);
        pblock->free = 1;
        pblock->region = MEM_REGION_PSRAM;
        pblock->next = NULL;
        pblock->prev = NULL;

        region_add_block(&regions[1], pblock);
        region_add_free(&regions[1], pblock);
        regions[1].free = pblock->size;
    }
}

static void *region_alloc(struct mem_region *r, size_t size) {
    heap_lock_acquire();

    struct heap_block *block = region_find_free(r, size);
    if (!block) {
        heap_lock_release();
        return NULL;
    }

    region_remove_free(r, block);
    region_split_block(r, block, size);

    block->free = 0;
    block->magic = HEAP_MAGIC;
    r->used += block->size + sizeof(struct heap_block);
    r->free -= block->size + sizeof(struct heap_block);

    heap_lock_release();
    return (uint8_t *)block + sizeof(struct heap_block);
}

void *kmalloc(size_t size) {
    if (size == 0) return NULL;

    size = (size + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1);

    void *ptr = region_alloc(&regions[MEM_REGION_INTERNAL], size);
    return ptr;
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

    heap_lock_acquire();

    struct heap_block *block = (struct heap_block *)((uint8_t *)ptr - sizeof(struct heap_block));
    int region_idx = block->region;

    if (region_idx < 0 || region_idx >= num_regions || block->magic != HEAP_MAGIC) {
        heap_lock_release();
        return;
    }

    if (block->free) {
        heap_lock_release();
        return;
    }

    struct mem_region *r = &regions[region_idx];
    block->free = 1;
    block->magic = HEAP_FREE_MAGIC;

    r->used -= block->size + sizeof(struct heap_block);
    r->free += block->size + sizeof(struct heap_block);

    region_coalesce(r, block);

    heap_lock_release();
}

void *kmalloc_aligned(size_t size, size_t align) {
    if (size == 0) return NULL;
    if (align == 0) align = HEAP_ALIGN;

    if ((align & (align - 1)) != 0) {
        return NULL;
    }

    heap_lock_acquire();

    for (int i = 0; i < num_regions; i++) {
        struct mem_region *r = &regions[i];
        struct heap_block *block = r->free_list;

        while (block) {
            if (block->free && block->size >= size) {
                uintptr_t block_addr = (uintptr_t)((uint8_t *)block + sizeof(struct heap_block));
                uintptr_t aligned_addr = (block_addr + align - 1) & ~(align - 1);
                size_t offset = aligned_addr - block_addr;

                if (block->size >= size + offset + sizeof(struct heap_block) + HEAP_MIN_BLOCK) {
                    if (offset > 0) {
                        struct heap_block *pre_block = (struct heap_block *)((uint8_t *)block + sizeof(struct heap_block) + offset - sizeof(struct heap_block));
                        pre_block->magic = HEAP_MAGIC;
                        pre_block->size = offset;
                        pre_block->free = 0;
                        pre_block->region = i;
                        pre_block->next = block->next;
                        pre_block->prev = block;
                        if (block->next) block->next->prev = pre_block;
                        block->next = pre_block;
                        block->size -= offset + sizeof(struct heap_block);
                        region_add_block(r, pre_block);
                        r->used += pre_block->size + sizeof(struct heap_block);
                        r->free -= pre_block->size + sizeof(struct heap_block);
                    }

                    region_remove_free(r, block);
                    region_split_block(r, block, size);
                    block->free = 0;
                    block->magic = HEAP_MAGIC;
                    r->used += block->size + sizeof(struct heap_block);
                    r->free -= block->size + sizeof(struct heap_block);

                    heap_lock_release();
                    return (uint8_t *)block + sizeof(struct heap_block);
                }
            }
            block = block->next;
        }
    }

    heap_lock_release();
    return NULL;
}

void *kmalloc_psram(size_t size) {
    if (size == 0) return NULL;
    if (!psram_available) return NULL;

    size = (size + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1);
    return region_alloc(&regions[MEM_REGION_PSRAM], size);
}

void *kzalloc_psram(size_t size) {
    void *ptr = kmalloc_psram(size);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void kfree_psram(void *ptr) {
    if (!ptr) return;
    kfree(ptr);
}

void *krealloc(void *ptr, size_t size) {
    if (!ptr) return kmalloc(size);
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    heap_lock_acquire();

    struct heap_block *block = (struct heap_block *)((uint8_t *)ptr - sizeof(struct heap_block));
    int region_idx = block->region;

    if (region_idx < 0 || region_idx >= num_regions || block->magic != HEAP_MAGIC || block->free) {
        heap_lock_release();
        return NULL;
    }

    size = (size + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1);

    if (block->size >= size) {
        if (block->size >= size + sizeof(struct heap_block) + HEAP_MIN_BLOCK) {
            region_split_block(&regions[region_idx], block, size);
        }
        heap_lock_release();
        return ptr;
    }

    struct mem_region *r = &regions[region_idx];
    struct heap_block *next = block->next;

    if (next && next->free && (block->size + sizeof(struct heap_block) + next->size) >= size) {
        region_remove_free(r, next);
        block->size += sizeof(struct heap_block) + next->size;
        if (block->size >= size + sizeof(struct heap_block) + HEAP_MIN_BLOCK) {
            region_split_block(r, block, size);
        }
        r->used += block->size;
        r->free -= block->size;
        heap_lock_release();
        return ptr;
    }

    heap_lock_release();

    void *new_ptr = kmalloc(size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, block->size);
        kfree(ptr);
    }
    return new_ptr;
}

void memory_stats(uint32_t *total, uint32_t *used, uint32_t *free) {
    *total = *used = *free = 0;
    for (int i = 0; i < num_regions; i++) {
        *total += regions[i].size;
        *used += regions[i].used;
        *free += regions[i].free;
    }
}

void memory_region_stats(int region, uint32_t *total, uint32_t *used, uint32_t *free, uint32_t *block_count) {
    if (region < 0 || region >= num_regions) {
        *total = *used = *free = *block_count = 0;
        return;
    }

    struct mem_region *r = &regions[region];
    *total = r->size;
    *used = r->used;
    *free = r->free;
    *block_count = 0;

    struct heap_block *block = r->block_list;
    while (block) {
        (*block_count)++;
        block = block->next;
    }
}

void memory_get_regions(struct mem_region_info *info, int max_regions) {
    int count = max_regions < num_regions ? max_regions : num_regions;
    for (int i = 0; i < count; i++) {
        info[i].start = regions[i].start;
        info[i].end = regions[i].end;
        info[i].size = regions[i].size;
        info[i].used = regions[i].used;
        info[i].free = regions[i].free;
        strncpy(info[i].name, regions[i].name, 15);
        info[i].name[15] = 0;
    }
}

int memory_check_heap(void) {
    for (int i = 0; i < num_regions; i++) {
        struct mem_region *r = &regions[i];
        struct heap_block *block = r->block_list;
        uint32_t total = 0;

        while (block) {
            if (block->magic != HEAP_MAGIC && block->magic != HEAP_FREE_MAGIC) {
                return -1;
            }
            if (block->size == 0 && block->free) {
                return -2;
            }
            if ((uint8_t *)block < (uint8_t *)r->start || (uint8_t *)block >= (uint8_t *)r->end) {
                return -3;
            }
            if (block->next && block->next->prev != block) {
                return -4;
            }
            if (block->prev && block->prev->next != block) {
                return -5;
            }
            total += block->size + sizeof(struct heap_block);
            block = block->next;
        }

        if (total != r->size) {
            return -6;
        }
    }
    return 0;
}

int psram_is_available(void) {
    return psram_available;
}

uint32_t psram_get_total(void) {
    if (!psram_available) return 0;
    return regions[MEM_REGION_PSRAM].size;
}

uint32_t psram_get_free(void) {
    if (!psram_available) return 0;
    return regions[MEM_REGION_PSRAM].free;
}

uint32_t psram_get_used(void) {
    if (!psram_available) return 0;
    return regions[MEM_REGION_PSRAM].used;
}

uint32_t internal_get_total(void) {
    return regions[MEM_REGION_INTERNAL].size;
}

uint32_t internal_get_free(void) {
    return regions[MEM_REGION_INTERNAL].free;
}

uint32_t internal_get_used(void) {
    return regions[MEM_REGION_INTERNAL].used;
}