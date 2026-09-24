#include <libc.h>
#include <kernel.h>

#define _IONBF 4

static struct _FILE stdio_files[3] = {
    {0, 0, NULL, 0, 0, 0, EOF, 0},
    {1, 0, NULL, 0, 0, 0, EOF, 0},
    {2, 0, NULL, 0, 0, 0, EOF, 0},
};

FILE *stdin = &stdio_files[0];
FILE *stdout = &stdio_files[1];
FILE *stderr = &stdio_files[2];

static int do_write(int fd, const void *buf, size_t count) {
    extern ssize_t vfs_write(struct file *, const void *, size_t);
    extern struct file *get_file(int fd);
    struct file *file = get_file(fd);
    if (!file) return -1;
    return vfs_write(file, buf, count);
}

static int do_read(int fd, void *buf, size_t count) {
    extern ssize_t vfs_read(struct file *, void *, size_t);
    extern struct file *get_file(int fd);
    struct file *file = get_file(fd);
    if (!file) return -1;
    return vfs_read(file, buf, count);
}

int fputc(int c, FILE *stream) {
    if (!stream) return EOF;
    if (stream->buf && stream->buf_pos < stream->buf_size) {
        stream->buf[stream->buf_pos++] = c;
        if (stream->flags & _IONBF || c == '\n' || stream->buf_pos == stream->buf_size) {
            if (do_write(stream->fd, stream->buf, stream->buf_pos) < 0) return EOF;
            stream->buf_pos = 0;
        }
    } else {
        char ch = c;
        if (do_write(stream->fd, &ch, 1) < 0) return EOF;
    }
    return c;
}

int fputs(const char *s, FILE *stream) {
    while (*s) {
        if (fputc(*s++, stream) == EOF) return EOF;
    }
    return 0;
}

int putchar(int c) {
    return fputc(c, stdout);
}

int puts(const char *s) {
    fputs(s, stdout);
    return putchar('\n');
}

int fgetc(FILE *stream) {
    if (!stream) return EOF;
    if (stream->ungetc_used) {
        stream->ungetc_used = 0;
        return stream->ungetc_buf;
    }
    if (stream->buf && stream->buf_pos < stream->buf_len) {
        return stream->buf[stream->buf_pos++];
    }
    char c;
    if (do_read(stream->fd, &c, 1) != 1) return EOF;
    return (unsigned char)c;
}

int getchar(void) {
    return fgetc(stdin);
}

int ungetc(int c, FILE *stream) {
    if (!stream || stream->ungetc_used || c == EOF) return EOF;
    stream->ungetc_buf = c;
    stream->ungetc_used = 1;
    return c;
}

char *fgets(char *s, int size, FILE *stream) {
    if (!stream || size <= 0) return NULL;
    int c = EOF;
    char *p = s;
    while (size > 1 && (c = fgetc(stream)) != EOF) {
        *p++ = c;
        size--;
        if (c == '\n') break;
    }
    *p = 0;
    return p == s && c == EOF ? NULL : s;
}

int fprintf(FILE *stream, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vfprintf(stream, format, ap);
    va_end(ap);
    return ret;
}

int vfprintf(FILE *stream, const char *format, va_list ap) {
    char buf[1024];
    int len = vsnprintf(buf, sizeof(buf), format, ap);
    if (len > 0) {
        fwrite(buf, 1, len, stream);
    }
    return len;
}

int printf(const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vfprintf(stdout, format, ap);
    va_end(ap);
    return ret;
}

int vprintf(const char *format, va_list ap) {
    return vfprintf(stdout, format, ap);
}

int snprintf(char *str, size_t size, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(str, size, format, ap);
    va_end(ap);
    return ret;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap) {
    char *p = str;
    char *end = str + size - 1;
    const char *f = format;

    while (*f && p < end) {
        if (*f == '%') {
            f++;
            if (*f == 's') {
                char *s = va_arg(ap, char *);
                if (!s) s = "(null)";
                while (*s && p < end) *p++ = *s++;
            } else if (*f == 'd' || *f == 'i') {
                int val = va_arg(ap, int);
                char num[16];
                int n = 0, neg = 0;
                if (val < 0) { neg = 1; val = -val; }
                do { num[n++] = '0' + val % 10; } while (val /= 10);
                if (neg && p < end) *p++ = '-';
                while (n-- && p < end) *p++ = num[n];
            } else if (*f == 'x') {
                unsigned int val = va_arg(ap, unsigned int);
                char num[16];
                int n = 0;
                do { int d = val & 0xF; num[n++] = d < 10 ? '0' + d : 'a' + d - 10; } while (val >>= 4);
                while (n-- && p < end) *p++ = num[n];
            } else if (*f == 'c') {
                if (p < end) *p++ = va_arg(ap, int);
            } else if (*f == '%') {
                if (p < end) *p++ = '%';
            } else {
                if (p < end) *p++ = *f;
            }
        } else {
            *p++ = *f;
        }
        f++;
    }
    *p = 0;
    return p - str;
}

size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t total = size * nmemb;
    size_t read = 0;
    char *p = (char *)ptr;
    while (read < total) {
        int c = fgetc(stream);
        if (c == EOF) break;
        *p++ = c;
        read++;
    }
    return read / size;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t total = size * nmemb;
    const char *p = (const char *)ptr;
    for (size_t i = 0; i < total; i++) {
        if (fputc(*p++, stream) == EOF) return i / size;
    }
    return nmemb;
}

int fseek(FILE *stream, long offset, int whence) {
    extern off_t vfs_lseek(struct file *, off_t, int);
    extern struct file *get_file(int fd);
    struct file *file = get_file(stream->fd);
    if (!file) return -1;
    off_t pos = vfs_lseek(file, offset, whence);
    if (pos < 0) return -1;
    stream->buf_pos = stream->buf_len = 0;
    return 0;
}

long ftell(FILE *stream) {
    (void)stream;
    return 0;
}

int fflush(FILE *stream) {
    if (!stream) return 0;
    if (stream->buf && stream->buf_pos > 0) {
        if (do_write(stream->fd, stream->buf, stream->buf_pos) < 0) return EOF;
        stream->buf_pos = 0;
    }
    return 0;
}

int feof(FILE *stream) {
    (void)stream;
    return 0;
}

int ferror(FILE *stream) {
    (void)stream;
    return 0;
}

FILE *fopen(const char *pathname, const char *mode) {
    int flags = 0;
    if (mode[0] == 'r') flags = O_RDONLY;
    else if (mode[0] == 'w') flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (mode[0] == 'a') flags = O_WRONLY | O_CREAT | O_APPEND;
    else return NULL;

    int fd = open(pathname, flags);
    if (fd < 0) return NULL;

    FILE *fp = kmalloc(sizeof(struct _FILE));
    if (!fp) { close(fd); return NULL; }
    fp->fd = fd;
    fp->flags = 0;
    fp->buf = NULL;
    fp->buf_size = 0;
    fp->buf_pos = 0;
    fp->buf_len = 0;
    fp->ungetc_buf = EOF;
    fp->ungetc_used = 0;
    return fp;
}

int fclose(FILE *stream) {
    if (!stream) return EOF;
    fflush(stream);
    close(stream->fd);
    if (stream->buf) kfree(stream->buf);
    kfree(stream);
    return 0;
}