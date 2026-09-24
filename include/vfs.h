#ifndef VFS_H
#define VFS_H

#include <kernel.h>

struct file;
struct dirent;
struct stat;

struct file_operations {
    int (*open)(struct file *, const char *, int);
    int (*close)(struct file *);
    ssize_t (*read)(struct file *, void *, size_t);
    ssize_t (*write)(struct file *, const void *, size_t);
    off_t (*lseek)(struct file *, off_t, int);
    ssize_t (*pread)(struct file *, void *, size_t, off_t);
    ssize_t (*pwrite)(struct file *, const void *, size_t, off_t);
    int (*ioctl)(struct file *, int, void *);
    int (*readdir)(struct file *, struct dirent *);
    int (*stat)(struct file *, struct stat *);
    int (*mkdir)(struct file *, const char *, mode_t);
    int (*rmdir)(struct file *, const char *);
    int (*unlink)(struct file *, const char *);
    int (*rename)(struct file *, const char *, const char *);
};

struct file {
    const struct file_operations *fops;
    void *private_data;
    off_t offset;
    int flags;
    int ref_count;
    char path[256];
};

struct dirent {
    uint32_t d_ino;
    uint8_t d_type;
    char d_name[256];
};

struct stat {
    dev_t st_dev;
    uint32_t st_ino;
    mode_t st_mode;
    uint32_t st_nlink;
    uid_t st_uid;
    gid_t st_gid;
    dev_t st_rdev;
    off_t st_size;
    time_t st_atime;
    time_t st_mtime;
    time_t st_ctime;
};

struct superblock {
    struct file_operations *fops;
    void *private_data;
    void *root;
    int flags;
};

struct mount {
    struct superblock *sb;
    char mountpoint[256];
    int flags;
};

#define MAX_MOUNTS 16
#define MAX_OPEN_FILES 128
#define PATH_MAX 256

#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT 0x40
#define O_TRUNC 0x200
#define O_APPEND 0x400
#define O_DIRECTORY 0x10000

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#define DT_REG 8
#define DT_DIR 4
#define DT_CHR 2
#define DT_BLK 6
#define DT_FIFO 1
#define DT_LNK 10
#define DT_SOCK 12

#define S_IFMT 0170000
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_IFCHR 0020000
#define S_IFBLK 0060000
#define S_IFIFO 0010000
#define S_IFLNK 0120000
#define S_IFSOCK 0140000

#define S_ISDIR(mode) (((mode) & S_IFMT) == S_IFDIR)
#define S_ISREG(mode) (((mode) & S_IFMT) == S_IFREG)
#define S_ISCHR(mode) (((mode) & S_IFMT) == S_IFCHR)

#define S_IRUSR 0400
#define S_IWUSR 0200
#define S_IXUSR 0100
#define S_IRGRP 0040
#define S_IWGRP 0020
#define S_IXGRP 0010
#define S_IROTH 0004
#define S_IWOTH 0002
#define S_IXOTH 0001

extern struct mount mounts[];
extern int mount_count;

struct file *vfs_open(const char *path, int flags);
int vfs_close(struct file *file);
ssize_t vfs_read(struct file *file, void *buf, size_t count);
ssize_t vfs_write(struct file *file, const void *buf, size_t count);
off_t vfs_lseek(struct file *file, off_t offset, int whence);
ssize_t vfs_pread(struct file *file, void *buf, size_t count, off_t offset);
ssize_t vfs_pwrite(struct file *file, const void *buf, size_t count, off_t offset);
int vfs_ioctl(struct file *file, int request, void *arg);
int vfs_readdir(struct file *file, struct dirent *ent);
int vfs_stat(const char *path, struct stat *st);
int vfs_fstat(struct file *file, struct stat *st);
int vfs_mkdir(const char *path, mode_t mode);
int vfs_rmdir(const char *path);
int vfs_unlink(const char *path);
int vfs_rename(const char *oldpath, const char *newpath);
int vfs_chdir(const char *path);
int vfs_getcwd(char *buf, size_t size);

int vfs_register_fs(const char *mountpoint, struct file_operations *fops, void *data);
int vfs_unregister_fs(const char *mountpoint);

int vfs_pipe(int *pipefd);

void fs_init(void);

#endif