#include <vfs.h>
#include <kernel.h>
#include <libc.h>
#include <string.h>
#include <stdlib.h>
#include <flashfs.h>

#define FLASH_SECTOR_SIZE 4096
#define FLASH_BLOCK_SIZE 4096

/* Forward declarations for flash driver functions */
extern int flash_read(uint32_t addr, void *buf, size_t len);
extern int flash_write(uint32_t addr, const void *buf, size_t len);
extern int flash_erase(uint32_t addr, size_t len);

/* Forward declaration of fops struct */
struct file_operations flashfs_fops;

static int flashfs_mounted = 0;

struct flashfs_sb_info flashfs_sb;

static inline uint32_t block_to_addr(uint32_t block) {
    return block * FLASH_BLOCK_SIZE;
}

static int flashfs_read_block(uint32_t block, void *buf) {
    return flash_read(block * FLASH_BLOCK_SIZE, buf, FLASH_BLOCK_SIZE);
}

static int flashfs_write_block(uint32_t block, const void *buf) {
    return flash_write(block * FLASH_BLOCK_SIZE, buf, FLASH_BLOCK_SIZE);
}

static int flashfs_read_superblock(void) {
    struct flashfs_superblock *sb = &flashfs_sb.sb;
    if (flash_read(0, sb, sizeof(struct flashfs_superblock)) != 0) {
        return -1;
    }
    if (sb->magic != FLASHFS_MAGIC) {
        return -1;
    }
    if (sb->version != FLASHFS_VERSION) {
        return -1;
    }
    return 0;
}

static int flashfs_write_superblock(void) {
    return flash_write(0, &flashfs_sb.sb, sizeof(struct flashfs_superblock));
}

static int flashfs_read_bitmap(void) {
    uint32_t bitmap_size = flashfs_sb.sb.total_blocks / 8;
    flashfs_sb.bitmap = kmalloc(bitmap_size);
    if (!flashfs_sb.bitmap) return -1;
    
    for (uint32_t i = 0; i < flashfs_sb.sb.bitmap_blocks; i++) {
        if (flashfs_read_block(FLASHFS_BITMAP_OFFSET + i, flashfs_sb.bitmap + i * FLASH_BLOCK_SIZE) != 0) {
            kfree(flashfs_sb.bitmap);
            return -1;
        }
    }
    return 0;
}

static int flashfs_write_bitmap(void) {
    for (uint32_t i = 0; i < flashfs_sb.sb.bitmap_blocks; i++) {
        if (flashfs_write_block(FLASHFS_BITMAP_OFFSET + i, flashfs_sb.bitmap + i * FLASH_BLOCK_SIZE) != 0) {
            return -1;
        }
    }
    return 0;
}

static int flashfs_write_inode(uint32_t ino, const struct flashfs_inode *inode) {
    if (ino == 0 || ino >= flashfs_sb.sb.inode_count) return -1;
    
    uint32_t inodes_per_block = FLASH_BLOCK_SIZE / sizeof(struct flashfs_inode);
    uint32_t block = FLASHFS_INODE_TABLE_OFFSET + ino / inodes_per_block;
    uint32_t index = ino % inodes_per_block;
    
    struct flashfs_inode *inode_table = kmalloc(FLASH_BLOCK_SIZE);
    if (!inode_table) return -1;
    
    if (flashfs_read_block(block, inode_table) != 0) {
        kfree(inode_table);
        return -1;
    }
    
    memcpy(&inode_table[index], inode, sizeof(struct flashfs_inode));
    
    int ret = flashfs_write_block(block, inode_table);
    kfree(inode_table);
    return ret;
}

int flashfs_format(void *flash_dev, uint32_t offset, uint32_t size) {
    (void)flash_dev;
    (void)offset;
    (void)size;
    
    struct flashfs_superblock sb = {0};
    sb.magic = FLASHFS_MAGIC;
    sb.version = FLASHFS_VERSION;
    sb.block_size = FLASH_BLOCK_SIZE;
    sb.total_blocks = 2048;
    sb.inode_count = 1024;
    sb.inode_table_blocks = 32;
    sb.bitmap_blocks = 1;
    sb.data_offset = FLASHFS_DATA_OFFSET;
    sb.free_blocks = 2048 - FLASHFS_DATA_OFFSET;
    sb.free_inodes = 1023;
    
    if (flash_write(0, &sb, sizeof(struct flashfs_superblock)) != 0) {
        return -1;
    }
    
    uint8_t *bitmap = kzalloc(256);
    if (!bitmap) return -1;
    
    for (int i = 0; i < FLASHFS_DATA_OFFSET; i++) {
        bitmap[i / 8] |= (1 << (i % 8));
    }
    
    if (flash_write(FLASH_BLOCK_SIZE, bitmap, FLASH_BLOCK_SIZE) != 0) {
        kfree(bitmap);
        return -1;
    }
    kfree(bitmap);
    
    struct flashfs_inode root_inode = {0};
    root_inode.type = FLASHFS_INODE_DIR;
    root_inode.nlink = 2;
    root_inode.size = 0;
    root_inode.atime = 0;
    root_inode.mtime = 0;
    root_inode.ctime = 0;
    
    if (flashfs_write_inode(1, &root_inode) != 0) {
        return -1;
    }
    
    return 0;
}

int flashfs_mount(const char *source, const char *target, int flags, void *data) {
    (void)source; (void)flags; (void)data;
    
    if (flashfs_read_superblock() != 0) {
        return -1;
    }
    
    if (flashfs_read_bitmap() != 0) {
        return -1;
    }
    
    flashfs_mounted = 1;
    return vfs_register_fs(target, &flashfs_fops, NULL);
}

int flashfs_unmount(const char *target) {
    (void)target;
    flashfs_write_bitmap();
    flashfs_write_superblock();
    if (flashfs_sb.bitmap) {
        kfree(flashfs_sb.bitmap);
        flashfs_sb.bitmap = NULL;
    }
    flashfs_mounted = 0;
    return 0;
}

int flashfs_open(struct file *file, const char *path, int flags) {
    (void)file; (void)path; (void)flags;
    return 0;
}

int flashfs_close(struct file *file) {
    (void)file;
    return 0;
}

ssize_t flashfs_read(struct file *file, void *buf, size_t count) {
    (void)file; (void)buf; (void)count;
    return 0;
}

ssize_t flashfs_write(struct file *file, const void *buf, size_t count) {
    (void)file; (void)buf; (void)count;
    return 0;
}

off_t flashfs_lseek(struct file *file, off_t offset, int whence) {
    (void)file; (void)offset; (void)whence;
    return 0;
}

int flashfs_stat(struct file *file, struct stat *st) {
    (void)file; (void)st;
    return 0;
}

int flashfs_readdir(struct file *file, struct dirent *ent) {
    (void)file; (void)ent;
    return 0;
}

int flashfs_mkdir(struct file *file, const char *path, mode_t mode) {
    (void)file; (void)path; (void)mode;
    return -1;
}

int flashfs_rmdir(struct file *file, const char *path) {
    (void)file; (void)path;
    return -1;
}

int flashfs_unlink(struct file *file, const char *path) {
    (void)file; (void)path;
    return -1;
}

int flashfs_rename(struct file *file, const char *oldpath, const char *newpath) {
    (void)file; (void)oldpath; (void)newpath;
    return -1;
}

int flashfs_chdir(struct file *file, const char *path) {
    (void)file; (void)path;
    return -1;
}

struct file_operations flashfs_fops = {
    .open = flashfs_open,
    .close = flashfs_close,
    .read = flashfs_read,
    .write = flashfs_write,
    .lseek = flashfs_lseek,
    .stat = flashfs_stat,
    .readdir = flashfs_readdir,
    .mkdir = flashfs_mkdir,
    .rmdir = flashfs_rmdir,
    .unlink = flashfs_unlink,
    .rename = flashfs_rename,
    .chdir = flashfs_chdir,
};