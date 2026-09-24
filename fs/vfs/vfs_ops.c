#include <kernel.h>
#include <string.h>

int vfs_mkdir(const char *path, mode_t mode) {
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
    return -1;
}

#define O_DIRECTORY 0x10000