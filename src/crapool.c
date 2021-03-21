#include <stdlib.h>
#include "crapool.h"

#ifdef USE_PTHREAD
    #include <pthread.h>
#endif

static int lock_desc(struct crapool_desc* const desc);
static int unlock_desc(struct crapool_desc* const desc);

static int lock_desc(struct crapool_desc* const desc)
{
#ifdef USE_PTHREAD
    return pthread_mutex_lock(desc->mutex);
#else
    return 0;
#endif
}

static int unlock_desc(struct crapool_desc* const desc)
{
#ifdef USE_PTHREAD
    return pthread_mutex_unlock(desc->mutex);
#else
    return 0;
#endif
}

struct crapool_desc* crapool_create(size_t size, void* const mutex)
{
    size_t alloc_size;
    struct crapool_desc *desc;
    void *buf;

    if (size == 0) {
        return NULL;
    }
    
    alloc_size = 
        (size + sizeof(struct crapool_desc)) | (CRAPOOL_ALIGNMENT - 1) + 1;

    if ((desc = (struct crapool_desc*)malloc(alloc_size)) == NULL) {
        return NULL;
    }
    buf = desc + sizeof(struct crapool_desc);

    desc->next_free = buf;
    desc->free_space = size;
    desc->next = NULL;
    desc->tail = desc;

#ifdef USE_PTHREAD
    if (mutex != NULL) {
        desc->mutex = mutex;
    }
    else {
        if ((desc->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t))) == NULL) {
            free(desc);
            return NULL;
        }
        pthread_mutex_init(desc->mutex, NULL);
    }
#endif

    return desc;
}

void* crapool_alloc(struct crapool_desc* const desc, size_t size)
{
    struct crapool_desc *desc_avail;
    struct crapool_desc *curr_desc;
    void *space;

    if (size == 0) {
        return NULL;
    }

    lock_desc(desc);

    if (size < CRAPOOL_LIBC_ALLOC_THRESHOLD) {
        for (curr_desc = desc; curr_desc->next != NULL; 
                curr_desc = curr_desc->next) 
        {
            if (curr_desc->free_space >= size) {
                curr_desc->free_space -= size;
                space = curr_desc->next_free;
                curr_desc->next_free += size;
                return space;
            }
        }
    }

#ifdef USE_PTHREAD
    if ((desc_avail = crapool_create(size, desc->mutex)) == NULL) {
#else
    if ((desc_avail = crapool_create(size, NULL)) == NULL) {
#endif
        return NULL;
    }

    desc->tail->next = desc_avail;

    for (curr_desc = desc; curr_desc->next != NULL; 
            curr_desc = curr_desc->next) 
    {
        curr_desc->tail = desc_avail;
    }

    space = desc_avail->next_free;

    unlock_desc(desc);
    return space;
}

int crapool_destroy(struct crapool_desc* const desc)
{
    struct crapool_desc *curr;
    struct crapool_desc *next;

#ifdef USE_PTHREAD
    pthread_mutex_t *mtx = desc->mutex;
#endif

    lock_desc(desc);

    curr = desc;
    do {
        next = curr->next;
        free(curr);
        curr = next;
    } while(curr != NULL);

    unlock_desc(desc);

#ifdef USE_PTHREAD
    pthread_mutex_destroy(mtx);
#endif

    return 0;
}

