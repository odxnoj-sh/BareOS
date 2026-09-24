#include <kernel.h>
#include <string.h>

struct pipe {
    char buffer[4096];
    int read_pos;
    int write_pos;
    int readers;
    int writers;
    struct task *read_wait;
    struct task *write_wait;
};

struct pipe *pipe_create(void) {
    struct pipe *p = kzalloc(sizeof(struct pipe));
    if (!p) return NULL;
    p->readers = p->writers = 1;
    return p;
}

void pipe_destroy(struct pipe *p) {
    kfree(p);
}

int pipe_read(struct pipe *p, void *buf, size_t count) {
    if (!p) return -1;
    int available = p->write_pos - p->read_pos;
    if (available <= 0) {
        if (p->writers == 0) return 0;
        return -1;
    }
    int to_read = (count < (size_t)available) ? (int)count : available;
    memcpy(buf, p->buffer + p->read_pos, to_read);
    p->read_pos += to_read;
    if (p->read_pos == p->write_pos) {
        p->read_pos = p->write_pos = 0;
    }
    return to_read;
}

int pipe_write(struct pipe *p, const void *buf, size_t count) {
    if (!p) return -1;
    int space = sizeof(p->buffer) - p->write_pos;
    if (space <= 0) {
        if (p->readers == 0) return -1;
        return -1;
    }
    int to_write = (count < (size_t)space) ? (int)count : space;
    memcpy(p->buffer + p->write_pos, buf, to_write);
    p->write_pos += to_write;
    return to_write;
}

int pipe_close_read(struct pipe *p) {
    if (!p) return -1;
    p->readers--;
    if (p->readers == 0 && p->write_wait) {
        task_wake(p->write_wait);
    }
    return 0;
}

int pipe_close_write(struct pipe *p) {
    if (!p) return -1;
    p->writers--;
    if (p->writers == 0 && p->read_wait) {
        task_wake(p->read_wait);
    }
    return 0;
}