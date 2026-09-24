#include <vfs.h>
#include <kernel.h>
#include <libc.h>
#include <string.h>
#include <stdlib.h>

struct mount mounts[MAX_MOUNTS];
struct file open_files[MAX_OPEN_FILES];
int mount_count = 0;

struct file *file_alloc(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (open_files[i].ref_count == 0) {
            open_files[i].ref_count = 1;
            open_files[i].offset = 0;
            return &open_files[i];
        }
    }
    return NULL;
}

void file_free(struct file *file) {
    if (file) {
        file->ref_count = 0;
        file->fops = NULL;
        file->private_data = NULL;
    }
}

struct file *vfs_open(const char *path, int flags) {
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(path, m->mountpoint, len) == 0 && (path[len] == '/' || path[len] == '\0')) {
            struct file *file = file_alloc();
            if (!file) return NULL;
            file->flags = flags;
            strncpy(file->path, path, PATH_MAX - 1);
            if (m->sb->fops->open) {
                int ret = m->sb->fops->open(file, path + len, flags);
                if (ret < 0) {
                    file_free(file);
                    return NULL;
                }
            }
            return file;
        }
    }
    return NULL;
}

int vfs_close(struct file *file) {
    if (!file || file->ref_count == 0) return -1;
    if (file->fops && file->fops->close) {
        file->fops->close(file);
    }
    file_free(file);
    return 0;
}

ssize_t vfs_read(struct file *file, void *buf, size_t count) {
    if (!file || !file->fops || !file->fops->read) return -1;
    return file->fops->read(file, buf, count);
}

ssize_t vfs_write(struct file *file, const void *buf, size_t count) {
    if (!file || !file->fops || !file->fops->write) return -1;
    return file->fops->write(file, buf, count);
}

off_t vfs_lseek(struct file *file, off_t offset, int whence) {
    if (!file || !file->fops || !file->fops->lseek) return -1;
    return file->fops->lseek(file, offset, whence);
}

int vfs_ioctl(struct file *file, int request, void *arg) {
    if (!file || !file->fops || !file->fops->ioctl) return -1;
    return file->fops->ioctl(file, request, arg);
}

int vfs_readdir(struct file *file, struct dirent *ent) {
    if (!file || !file->fops || !file->fops->readdir) return -1;
    return file->fops->readdir(file, ent);
}

int vfs_stat(const char *path, struct stat *st) {
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(path, m->mountpoint, len) == 0 && (path[len] == '/' || path[len] == '\0')) {
            if (m->sb->fops->stat) {
                struct file *file = file_alloc();
                if (!file) return -1;
                int ret = m->sb->fops->stat(file, st);
                file_free(file);
                return ret;
            }
        }
    }
    return -1;
}

void fs_init(void) {
    memset(mounts, 0, sizeof(mounts));
    memset(open_files, 0, sizeof(open_files));
    mount_count = 0;

    extern int ramfs_mount(const char *source, const char *target, int flags, void *data);
    ramfs_mount(NULL, "/", 0, NULL);

    extern int devfs_mount(const char *source, const char *target, int flags, void *data);
    devfs_mount(NULL, "/dev", 0, NULL);
}

#define O_CREAT 0x40
#define O_DIRECTORY 0x10000

int vfs_mkdir(const char *path, mode_t mode) {
    (void)mode;
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(path, m->mountpoint, len) == 0 && (path[len] == '/' || path[len] == '\0')) {
            struct file *file = file_alloc();
            if (!file) return -1;
            if (m->sb->fops->open) {
                int ret = m->sb->fops->open(file, path + len, O_CREAT | O_DIRECTORY);
                file_free(file);
                return ret;
            }
        }
    }
    return -1;
}

int vfs_rmdir(const char *path) {
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(path, m->mountpoint, len) == 0 && (path[len] == '/' || path[len] == '\0')) {
            struct file *file = file_alloc();
            if (!file) return -1;
            if (m->sb->fops->ioctl) {
                int ret = m->sb->fops->ioctl(file, 0, (void *)path);
                file_free(file);
                return ret;
            }
        }
    }
    return -1;
}

int vfs_unlink(const char *path) {
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(path, m->mountpoint, len) == 0 && (path[len] == '/' || path[len] == '\0')) {
            struct file *file = file_alloc();
            if (!file) return -1;
            if (m->sb->fops->ioctl) {
                int ret = m->sb->fops->ioctl(file, 1, (void *)path);
                file_free(file);
                return ret;
            }
        }
    }
    return -1;
}

int vfs_rename(const char *oldpath, const char *newpath) {
    (void)oldpath; (void)newpath;
    return -1;
}

int vfs_register_fs(const char *mountpoint, struct file_operations *fops, void *data) {
    if (mount_count >= MAX_MOUNTS) return -1;
    struct mount *m = &mounts[mount_count++];
    m->sb = kmalloc(sizeof(struct superblock));
    if (!m->sb) return -1;
    m->sb->fops = fops;
    m->sb->private_data = data;
    m->sb->root = NULL;
    strncpy(m->mountpoint, mountpoint, PATH_MAX - 1);
    return 0;
}