#include <vfs.h>
#include <kernel.h>
#include <libc.h>
#include <string.h>
#include <stdlib.h>

struct mount mounts[MAX_MOUNTS];
struct file open_files[MAX_OPEN_FILES];
int mount_count = 0;

static char cwd_buf[256] = "/";

static int path_normalize(const char *path, char *out, size_t out_size) {
    if (!path || !out || out_size == 0) return -1;
    
    const char *p = path;
    char *out_p = out;
    size_t remaining = out_size;
    
    if (*p != '/') {
        const char *cwd = cwd_buf;
        size_t cwd_len = strlen(cwd);
        if (cwd_len + 1 >= out_size) return -1;
        memcpy(out_p, cwd, cwd_len);
        out_p += cwd_len;
        *out_p++ = '/';
        remaining -= cwd_len + 1;
    }
    
    while (*p && remaining > 1) {
        while (*p == '/') p++;
        if (!*p) break;
        
        const char *end = p;
        while (*end && *end != '/') end++;
        size_t len = end - p;
        
        if (len == 1 && p[0] == '.') {
            p = end;
            continue;
        }
        
        if (len == 2 && p[0] == '.' && p[1] == '.') {
            if (out_p > out + 1) {
                out_p--;
                while (out_p > out && *out_p != '/') out_p--;
            }
            p = end;
            continue;
        }
        
        if (len + 1 >= remaining) return -1;
        *out_p++ = '/';
        memcpy(out_p, p, len);
        out_p += len;
        remaining -= len + 1;
        p = end;
    }
    
    if (out_p == out) {
        *out_p++ = '/';
    }
    *out_p = '\0';
    return 0;
}

struct file *file_alloc(void) {
    for (int i = 0; i < MAX_OPEN_FILES; i++) {
        if (open_files[i].ref_count == 0) {
            open_files[i].ref_count = 1;
            open_files[i].offset = 0;
            open_files[i].flags = 0;
            open_files[i].fops = NULL;
            open_files[i].private_data = NULL;
            open_files[i].path[0] = '\0';
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
        file->offset = 0;
        file->flags = 0;
        file->path[0] = '\0';
    }
}

struct file *vfs_open(const char *path, int flags) {
    char norm_path[PATH_MAX];
    if (path_normalize(path, norm_path, PATH_MAX) < 0) return NULL;
    
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(norm_path, m->mountpoint, len) == 0 && (norm_path[len] == '/' || norm_path[len] == '\0')) {
            struct file *file = file_alloc();
            if (!file) return NULL;
            file->flags = flags;
            strncpy(file->path, norm_path, PATH_MAX - 1);
            file->path[PATH_MAX - 1] = '\0';
            if (m->sb->fops->open) {
                int ret = m->sb->fops->open(file, norm_path + len, flags);
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

ssize_t vfs_pread(struct file *file, void *buf, size_t count, off_t offset) {
    if (!file || !file->fops) return -1;
    if (file->fops->pread) {
        return file->fops->pread(file, buf, count, offset);
    }
    off_t old_offset = file->offset;
    off_t new_off = vfs_lseek(file, offset, SEEK_SET);
    if (new_off < 0) return -1;
    ssize_t ret = vfs_read(file, buf, count);
    vfs_lseek(file, old_offset, SEEK_SET);
    return ret;
}

ssize_t vfs_pwrite(struct file *file, const void *buf, size_t count, off_t offset) {
    if (!file || !file->fops) return -1;
    if (file->fops->pwrite) {
        return file->fops->pwrite(file, buf, count, offset);
    }
    off_t old_offset = file->offset;
    off_t new_off = vfs_lseek(file, offset, SEEK_SET);
    if (new_off < 0) return -1;
    ssize_t ret = vfs_write(file, buf, count);
    vfs_lseek(file, old_offset, SEEK_SET);
    return ret;
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
    char norm_path[PATH_MAX];
    if (path_normalize(path, norm_path, PATH_MAX) < 0) return -1;
    
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(norm_path, m->mountpoint, len) == 0 && (norm_path[len] == '/' || norm_path[len] == '\0')) {
            if (m->sb->fops->stat) {
                struct file *file = file_alloc();
                if (!file) return -1;
                strncpy(file->path, norm_path, PATH_MAX - 1);
                int ret = m->sb->fops->stat(file, st);
                file_free(file);
                return ret;
            }
        }
    }
    return -1;
}

int vfs_fstat(struct file *file, struct stat *st) {
    if (!file || !file->fops || !file->fops->stat) return -1;
    return file->fops->stat(file, st);
}

void fs_init(void) {
    memset(mounts, 0, sizeof(mounts));
    memset(open_files, 0, sizeof(open_files));
    mount_count = 0;
    cwd_buf[0] = '/'; cwd_buf[1] = '\0';

    extern int ramfs_mount(const char *source, const char *target, int flags, void *data);
    ramfs_mount(NULL, "/", 0, NULL);

    extern int devfs_mount(const char *source, const char *target, int flags, void *data);
    devfs_mount(NULL, "/dev", 0, NULL);
}

#define O_CREAT 0x40
#define O_DIRECTORY 0x10000
#define O_TRUNC 0x200

int vfs_mkdir(const char *path, mode_t mode) {
    char norm_path[PATH_MAX];
    if (path_normalize(path, norm_path, PATH_MAX) < 0) return -1;
    
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(norm_path, m->mountpoint, len) == 0 && (norm_path[len] == '/' || norm_path[len] == '\0')) {
            struct file *file = file_alloc();
            if (!file) return -1;
            if (m->sb->fops->mkdir) {
                int ret = m->sb->fops->mkdir(file, norm_path + len, mode);
                file_free(file);
                return ret;
            }
        }
    }
    return -1;
}

int vfs_rmdir(const char *path) {
    char norm_path[PATH_MAX];
    if (path_normalize(path, norm_path, PATH_MAX) < 0) return -1;
    
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(norm_path, m->mountpoint, len) == 0 && (norm_path[len] == '/' || norm_path[len] == '\0')) {
            struct file *file = file_alloc();
            if (!file) return -1;
            if (m->sb->fops->rmdir) {
                int ret = m->sb->fops->rmdir(file, norm_path + len);
                file_free(file);
                return ret;
            }
        }
    }
    return -1;
}

int vfs_unlink(const char *path) {
    char norm_path[PATH_MAX];
    if (path_normalize(path, norm_path, PATH_MAX) < 0) return -1;
    
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(norm_path, m->mountpoint, len) == 0 && (norm_path[len] == '/' || norm_path[len] == '\0')) {
            struct file *file = file_alloc();
            if (!file) return -1;
            if (m->sb->fops->unlink) {
                int ret = m->sb->fops->unlink(file, norm_path + len);
                file_free(file);
                return ret;
            }
        }
    }
    return -1;
}

int vfs_rename(const char *oldpath, const char *newpath) {
    char norm_old[PATH_MAX], norm_new[PATH_MAX];
    if (path_normalize(oldpath, norm_old, PATH_MAX) < 0) return -1;
    if (path_normalize(newpath, norm_new, PATH_MAX) < 0) return -1;
    
    for (int i = mount_count - 1; i >= 0; i--) {
        struct mount *m = &mounts[i];
        size_t len = strlen(m->mountpoint);
        if (strncmp(norm_old, m->mountpoint, len) == 0 && (norm_old[len] == '/' || norm_old[len] == '\0') &&
            strncmp(norm_new, m->mountpoint, len) == 0 && (norm_new[len] == '/' || norm_new[len] == '\0')) {
            if (m->sb->fops->rename) {
                return m->sb->fops->rename(NULL, norm_old + len, norm_new + len);
            }
        }
    }
    return -1;
}

int vfs_chdir(const char *path) {
    char norm_path[PATH_MAX];
    if (path_normalize(path, norm_path, PATH_MAX) < 0) return -1;
    
    struct file *file = vfs_open(norm_path, O_RDONLY | O_DIRECTORY);
    if (!file) return -1;
    
    if (file->fops && file->fops->stat) {
        struct stat st;
        if (file->fops->stat(file, &st) == 0 && S_ISDIR(st.st_mode)) {
            strncpy(cwd_buf, norm_path, 255);
            cwd_buf[255] = '\0';
            vfs_close(file);
            return 0;
        }
    }
    vfs_close(file);
    return -1;
}

int vfs_getcwd(char *buf, size_t size) {
    size_t len = strlen(cwd_buf);
    if (len + 1 > size) return -1;
    memcpy(buf, cwd_buf, len + 1);
    return 0;
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

int vfs_unregister_fs(const char *mountpoint) {
    for (int i = 0; i < mount_count; i++) {
        if (strcmp(mounts[i].mountpoint, mountpoint) == 0) {
            if (mounts[i].sb) kfree(mounts[i].sb);
            for (int j = i; j < mount_count - 1; j++) {
                mounts[j] = mounts[j + 1];
            }
            mount_count--;
            return 0;
        }
    }
    return -1;
}

int vfs_pipe(int *pipefd) {
    (void)pipefd;
    return -1;
}