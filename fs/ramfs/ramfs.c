#include <vfs.h>
#include <kernel.h>
#include <libc.h>
#include <string.h>
#include <stdlib.h>

#define RAMFS_MAX_FILES 64
#define RAMFS_MAX_DATA 8192

struct ramfs_node {
    char name[256];
    mode_t mode;
    uint32_t size;
    uint8_t *data;
    struct ramfs_node *parent;
    struct ramfs_node *children;
    struct ramfs_node *next;
    struct ramfs_node *prev;
    time_t atime;
    time_t mtime;
    time_t ctime;
};

struct ramfs_sb {
    struct ramfs_node *root;
    uint32_t file_count;
    uint32_t total_size;
};

static struct ramfs_node ramfs_nodes[RAMFS_MAX_FILES];
static uint8_t ramfs_data_pool[RAMFS_MAX_DATA];
static uint32_t ramfs_node_count = 0;
static uint32_t ramfs_data_used = 0;
static struct ramfs_sb ramfs_superblock;

struct ramfs_node *ramfs_alloc_node(void) {
    if (ramfs_node_count >= RAMFS_MAX_FILES) return NULL;
    struct ramfs_node *node = &ramfs_nodes[ramfs_node_count++];
    memset(node, 0, sizeof(struct ramfs_node));
    node->atime = node->mtime = node->ctime = 0;
    return node;
}

void ramfs_free_node(struct ramfs_node *node) {
    if (node->data) {
        ramfs_data_used -= node->size;
        node->data = NULL;
    }
    if (node->prev) node->prev->next = node->next;
    if (node->next) node->next->prev = node->prev;
    if (node->parent && node->parent->children == node) {
        node->parent->children = node->next;
    }
    node->parent = node->children = node->next = node->prev = NULL;
}

struct ramfs_node *ramfs_find_child(struct ramfs_node *parent, const char *name) {
    for (struct ramfs_node *child = parent->children; child; child = child->next) {
        if (strcmp(child->name, name) == 0) return child;
    }
    return NULL;
}

static struct ramfs_node *ramfs_resolve_parent(const char *path, char *basename) {
    struct ramfs_node *node = ramfs_superblock.root;
    const char *p = path;
    char name[256];

    while (*p == '/') p++;
    while (*p) {
        const char *end = p;
        while (*end && *end != '/') end++;
        size_t len = end - p;
        if (len == 0) break;

        if (len >= 256) len = 255;
        memcpy(name, p, len);
        name[len] = 0;

        struct ramfs_node *child = ramfs_find_child(node, name);
        if (!child) return NULL;

        while (*end == '/') end++;
        p = end;
        if (*p == '\0') {
            if (basename) strncpy(basename, name, 255);
            return node;
        }
        node = child;
        p = end;
    }
    return node;
}

int ramfs_open(struct file *file, const char *path, int flags) {
    struct ramfs_sb *sb = (struct ramfs_sb *)file->private_data;
    if (!sb) sb = &ramfs_superblock;

    struct ramfs_node *node = sb->root;
    const char *p = path;

    while (*p == '/') p++;
    while (*p) {
        const char *end = p;
        while (*end && *end != '/') end++;
        size_t len = end - p;
        if (len == 0) break;

        char name[256];
        if (len >= 256) len = 255;
        memcpy(name, p, len);
        name[len] = 0;

        node = ramfs_find_child(node, name);
        if (!node) {
            if (flags & O_CREAT) {
                node = ramfs_alloc_node();
                if (!node) return -1;
                strncpy(node->name, name, 255);
                node->mode = S_IFREG | 0644;
                node->parent = sb->root;
                node->next = sb->root->children;
                if (sb->root->children) sb->root->children->prev = node;
                sb->root->children = node;
            } else {
                return -1;
            }
        }
        while (*end == '/') end++;
        p = end;
    }

    file->private_data = node;
    return 0;
}

int ramfs_close(struct file *file) {
    (void)file;
    return 0;
}

ssize_t ramfs_read(struct file *file, void *buf, size_t count) {
    struct ramfs_node *node = (struct ramfs_node *)file->private_data;
    if (!node || !node->data) return 0;
    if ((uint32_t)file->offset >= node->size) return 0;
    size_t to_read = count;
    if ((uint32_t)file->offset + to_read > node->size) to_read = node->size - (uint32_t)file->offset;
    memcpy(buf, node->data + file->offset, to_read);
    file->offset += to_read;
    return to_read;
}

ssize_t ramfs_write(struct file *file, const void *buf, size_t count) {
    struct ramfs_node *node = (struct ramfs_node *)file->private_data;
    if (!node) return -1;

    if (!node->data) {
        if (ramfs_data_used + count > RAMFS_MAX_DATA) return -1;
        node->data = &ramfs_data_pool[ramfs_data_used];
        ramfs_data_used += count;
    } else if ((uint32_t)file->offset + count > node->size) {
        if (ramfs_data_used + count > RAMFS_MAX_DATA) return -1;
        uint8_t *new_data = &ramfs_data_pool[ramfs_data_used];
        memcpy(new_data, node->data, node->size);
        node->data = new_data;
        ramfs_data_used += count;
    }

    memcpy(node->data + file->offset, buf, count);
    file->offset += count;
    if ((uint32_t)file->offset > node->size) node->size = (uint32_t)file->offset;
    return count;
}

off_t ramfs_lseek(struct file *file, off_t offset, int whence) {
    struct ramfs_node *node = (struct ramfs_node *)file->private_data;
    if (!node) return -1;
    off_t new_offset;
    switch (whence) {
        case SEEK_SET: new_offset = offset; break;
        case SEEK_CUR: new_offset = file->offset + offset; break;
        case SEEK_END: new_offset = (off_t)node->size + offset; break;
        default: return -1;
    }
    if (new_offset < 0) return -1;
    file->offset = new_offset;
    return new_offset;
}

ssize_t ramfs_pread(struct file *file, void *buf, size_t count, off_t offset) {
    struct ramfs_node *node = (struct ramfs_node *)file->private_data;
    if (!node || !node->data) return 0;
    if ((uint32_t)offset >= node->size) return 0;
    size_t to_read = count;
    if ((uint32_t)offset + to_read > node->size) to_read = node->size - (uint32_t)offset;
    memcpy(buf, node->data + offset, to_read);
    return to_read;
}

ssize_t ramfs_pwrite(struct file *file, const void *buf, size_t count, off_t offset) {
    struct ramfs_node *node = (struct ramfs_node *)file->private_data;
    if (!node) return -1;

    if (!node->data) {
        if (ramfs_data_used + count > RAMFS_MAX_DATA) return -1;
        node->data = &ramfs_data_pool[ramfs_data_used];
        ramfs_data_used += count;
    } else if ((uint32_t)offset + count > node->size) {
        if (ramfs_data_used + count > RAMFS_MAX_DATA) return -1;
        uint8_t *new_data = &ramfs_data_pool[ramfs_data_used];
        memcpy(new_data, node->data, node->size);
        node->data = new_data;
        ramfs_data_used += count;
    }

    memcpy(node->data + offset, buf, count);
    if ((uint32_t)offset + count > node->size) node->size = (uint32_t)offset + count;
    return count;
}

int ramfs_stat(struct file *file, struct stat *st) {
    struct ramfs_node *node = (struct ramfs_node *)file->private_data;
    if (!node) return -1;
    memset(st, 0, sizeof(struct stat));
    st->st_mode = node->mode;
    st->st_size = node->size;
    st->st_atime = node->atime;
    st->st_mtime = node->mtime;
    st->st_ctime = node->ctime;
    return 0;
}

int ramfs_readdir(struct file *file, struct dirent *ent) {
    struct ramfs_node *node = (struct ramfs_node *)file->private_data;
    struct ramfs_node *iter = (struct ramfs_node *)file->private_data;
    
    if (!iter) return 0;
    
    if (iter->children) {
        iter = iter->children;
    } else {
        while (iter && !iter->next) {
            if (iter == node) return 0;
            iter = iter->parent;
        }
        if (!iter) return 0;
        iter = iter->next;
    }
    
    if (!iter) return 0;
    
    file->private_data = iter;
    
    ent->d_ino = 0;
    strncpy(ent->d_name, iter->name, 255);
    ent->d_type = (iter->mode & S_IFDIR) ? DT_DIR : DT_REG;
    return 1;
}

int ramfs_mkdir(struct file *file, const char *path, mode_t mode) {
    (void)mode;
    struct ramfs_sb *sb = (struct ramfs_sb *)file->private_data;
    if (!sb) sb = &ramfs_superblock;

    struct ramfs_node *node = sb->root;
    const char *p = path;
    char name[256];

    while (*p == '/') p++;
    while (*p) {
        const char *end = p;
        while (*end && *end != '/') end++;
        size_t len = end - p;
        if (len == 0) break;

        if (len >= 256) len = 255;
        memcpy(name, p, len);
        name[len] = 0;

        struct ramfs_node *child = ramfs_find_child(node, name);
        if (!child) {
            child = ramfs_alloc_node();
            if (!child) return -1;
            strncpy(child->name, name, 255);
            child->mode = S_IFDIR | 0755;
            child->parent = node;
            child->next = node->children;
            if (node->children) node->children->prev = child;
            node->children = child;
        }
        node = child;
        while (*end == '/') end++;
        p = end;
    }

    if (!node || !S_ISDIR(node->mode)) return -1;

    struct ramfs_node *child = ramfs_find_child(node, name);
    if (child) return -1;

    child = ramfs_alloc_node();
    if (!child) return -1;
    strncpy(child->name, name, 255);
    child->mode = S_IFDIR | 0755;
    child->parent = node;
    child->next = node->children;
    if (node->children) node->children->prev = child;
    node->children = child;
    return 0;
}

int ramfs_rmdir(struct file *file, const char *path) {
    struct ramfs_sb *sb = (struct ramfs_sb *)file->private_data;
    if (!sb) sb = &ramfs_superblock;

    struct ramfs_node *node = sb->root;
    const char *p = path;
    char name[256];

    while (*p == '/') p++;
    while (*p) {
        const char *end = p;
        while (*end && *end != '/') end++;
        size_t len = end - p;
        if (len == 0) break;

        if (len >= 256) len = 255;
        memcpy(name, p, len);
        name[len] = 0;

        node = ramfs_find_child(node, name);
        if (!node) return -1;
        
        while (*end == '/') end++;
        p = end;
    }

    if (!node || !S_ISDIR(node->mode)) return -1;
    if (node->children) return -1;

    ramfs_free_node(node);
    return 0;
}

int ramfs_unlink(struct file *file, const char *path) {
    struct ramfs_sb *sb = (struct ramfs_sb *)file->private_data;
    if (!sb) sb = &ramfs_superblock;

    struct ramfs_node *node = sb->root;
    const char *p = path;
    char name[256];

    while (*p == '/') p++;
    while (*p) {
        const char *end = p;
        while (*end && *end != '/') end++;
        size_t len = end - p;
        if (len == 0) break;

        if (len >= 256) len = 255;
        memcpy(name, p, len);
        name[len] = 0;

        struct ramfs_node *child = ramfs_find_child(node, name);
        if (!child) return -1;
        
        if (*end == '\0') {
            if (S_ISDIR(child->mode)) return -1;
            ramfs_free_node(child);
            return 0;
        }
        
        node = child;
        while (*end == '/') end++;
        p = end;
    }
    return -1;
}

int ramfs_rename(struct file *file, const char *oldpath, const char *newpath) {
    (void)file;
    struct ramfs_sb *sb = &ramfs_superblock;

    struct ramfs_node *node = sb->root;
    const char *p = oldpath;
    char old_name[256];
    struct ramfs_node *old_node = NULL;

    while (*p == '/') p++;
    while (*p) {
        const char *end = p;
        while (*end && *end != '/') end++;
        size_t len = end - p;
        if (len == 0) break;

        if (len >= 256) len = 255;
        char name[256];
        memcpy(name, p, len);
        name[len] = 0;

        struct ramfs_node *child = ramfs_find_child(node, name);
        if (!child) return -1;

        if (*end == '\0') {
            strncpy(old_name, name, 255);
            old_node = child;
            break;
        }

        node = child;
        while (*end == '/') end++;
        p = end;
    }

    if (!old_node || old_node == ramfs_superblock.root) return -1;

    char new_basename[256];
    struct ramfs_node *new_parent = ramfs_resolve_parent(newpath, new_basename);
    if (!new_parent) return -1;

    if (strcmp(old_name, new_basename) == 0 && new_parent == old_node->parent) return 0;

    if (new_parent == old_node || (S_ISDIR(old_node->mode) && old_node == new_parent)) {
        return -1;
    }

    struct ramfs_node *existing = ramfs_find_child(new_parent, new_basename);
    if (existing) {
        if (S_ISDIR(existing->mode)) {
            if (existing->children) return -1;
            ramfs_free_node(existing);
        } else {
            ramfs_free_node(existing);
        }
    }

    if (old_node->parent) {
        if (old_node->prev) old_node->prev->next = old_node->next;
        if (old_node->next) old_node->next->prev = old_node->prev;
        if (old_node->parent->children == old_node) {
            old_node->parent->children = old_node->next;
        }
    }

    strncpy(old_node->name, new_basename, 255);
    old_node->parent = new_parent;
    old_node->next = new_parent->children;
    old_node->prev = NULL;
    if (new_parent->children) new_parent->children->prev = old_node;
    new_parent->children = old_node;

    return 0;
}

struct file_operations ramfs_fops = {
    .open = ramfs_open,
    .close = ramfs_close,
    .read = ramfs_read,
    .write = ramfs_write,
    .lseek = ramfs_lseek,
    .pread = ramfs_pread,
    .pwrite = ramfs_pwrite,
    .stat = ramfs_stat,
    .readdir = ramfs_readdir,
    .mkdir = ramfs_mkdir,
    .rmdir = ramfs_rmdir,
    .unlink = ramfs_unlink,
    .rename = ramfs_rename,
};

int ramfs_mount(const char *source, const char *target, int flags, void *data) {
    (void)source; (void)flags; (void)data;
    ramfs_superblock.root = ramfs_alloc_node();
    if (!ramfs_superblock.root) return -1;
    strncpy(ramfs_superblock.root->name, "/", 255);
    ramfs_superblock.root->mode = S_IFDIR | 0755;
    ramfs_superblock.file_count = 1;
    ramfs_superblock.total_size = 0;

    ramfs_superblock.root->children = ramfs_alloc_node();
    if (ramfs_superblock.root->children) {
        strncpy(ramfs_superblock.root->children->name, "bin", 255);
        ramfs_superblock.root->children->mode = S_IFDIR | 0755;
        ramfs_superblock.root->children->parent = ramfs_superblock.root;
    }

    return vfs_register_fs(target, &ramfs_fops, &ramfs_superblock);
}