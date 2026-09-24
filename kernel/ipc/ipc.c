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