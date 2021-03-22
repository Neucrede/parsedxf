#ifndef __CRAPOOL_H__
#define __CRAPOOL_H__

#include <sys/types.h>

#ifdef USE_PTHREAD
    #include <pthread.h>
#endif

#define CRAPOOL_LIBC_ALLOC_THRESHOLD 1024000
#define CRAPOOL_SIZE_MULTIPLE (1 << 3)
#define CRAPOOL_SIZE_MIN 4096

struct crapool_desc;
struct crapool_desc {
    size_t free_space;
    void *next_free;

    struct crapool_desc *next;
    struct crapool_desc *tail;
#ifdef USE_PTHREAD
    pthread_mutex_t *mutex;
#endif
};

struct crapool_desc* crapool_create(size_t size, void* const mutex);
void* crapool_alloc(struct crapool_desc* const desc, size_t size);
int crapool_destroy(struct crapool_desc* const desc);

#endif /* __CRAPOOL_H__ */
