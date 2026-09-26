#ifndef FLASHFS_H
#define FLASHFS_H

#include <vfs.h>

#define FLASHFS_MAGIC 0x46534146
#define FLASHFS_VERSION 1

#define FLASHFS_BLOCK_SIZE 4096
#define FLASHFS_INODE_SIZE 128
#define FLASHFS_MAX_INODES 1024
#define FLASHFS_MAX_BLOCKS 2048
#define FLASHFS_INODE_TABLE_BLOCKS 32
#define FLASHFS_BITMAP_BLOCKS 1

#define FLASHFS_INODE_FILE 1
#define FLASHFS_INODE_DIR 2

#define FLASHFS_INODE_TABLE_OFFSET 1
#define FLASHFS_BITMAP_OFFSET (FLASHFS_INODE_TABLE_OFFSET + FLASHFS_INODE_TABLE_BLOCKS)
#define FLASHFS_DATA_OFFSET (FLASHFS_BITMAP_OFFSET + FLASHFS_BITMAP_BLOCKS)

struct flashfs_superblock {
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t inode_count;
    uint32_t inode_table_blocks;
    uint32_t bitmap_blocks;
    uint32_t data_offset;
    uint32_t free_blocks;
    uint32_t free_inodes;
};

struct flashfs_inode {
    uint16_t type;
    uint16_t nlink;
    uint32_t size;
    uint32_t blocks[12];
    uint32_t indirect;
    uint32_t atime;
    uint32_t mtime;
    uint32_t ctime;
};

struct flashfs_dirent {
    uint32_t inode;
    uint8_t type;
    char name[252];
};

struct flashfs_sb_info {
    struct flashfs_superblock sb;
    uint8_t *bitmap;
    uint32_t bitmap_blocks;
};

int flashfs_format(void *flash_dev, uint32_t offset, uint32_t size);
int flashfs_mount(const char *source, const char *target, int flags, void *data);
int flashfs_unmount(const char *target);

int flashfs_open(struct file *file, const char *path, int flags);
int ramfs_close(struct file *file);
ssize_t flashfs_read(struct file *file, void *buf, size_t count);
ssize_t ramfs_write(struct file *file, const void *buf, size_t count);
off_t ramfs_lseek(struct file *file, off_t offset, int whence);
int ramfs_stat(struct file *file, struct stat *st);
int ramfs_readdir(struct file *file, struct dirent *ent);
int ramfs_mkdir(struct file *file, const char *path, int flags);
int ramfs_rmdir(struct file *file, const char *path);
int ramfs_unlink(struct file *file, const char *path);
int ramfs_rename(struct file *file, const char *oldpath, const char *newpath);
int ramfs_chdir(struct file *file, const char *path);

#endif