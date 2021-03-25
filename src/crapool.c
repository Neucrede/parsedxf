#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "crapool.h"
#include "dbgprint.h"

#define NO_DBGPRINT

#ifdef USE_PTHREAD
    #include <pthread.h>
#endif

static int lock_desc(struct crapool_desc* const desc)
{
#if defined(USE_PTHREAD) && defined(CRAPOOL_LOCK_ENABLED)
#if (CRAPOOL_LOCK_ENABLED == 1)
    return pthread_mutex_lock(desc->mutex);
#endif
#else
    return 0;
#endif
}

static int unlock_desc(struct crapool_desc* const desc)
{
#if defined(USE_PTHREAD) && defined(CRAPOOL_LOCK_ENABLED)
#if (CRAPOOL_LOCK_ENABLED == 1)
    return pthread_mutex_unlock(desc->mutex);
#endif
#else
    return 0;
#endif
}

struct crapool_desc* crapool_create(size_t size, void* const mutex)
{
    size_t alloc_size;
    struct crapool_desc *desc;
    char *buf;

    if (size < CRAPOOL_SIZE_MIN) {
        size = CRAPOOL_SIZE_MIN;
    }
    
    alloc_size = 
        (size + sizeof(struct crapool_desc)) | (CRAPOOL_SIZE_MULTIPLE - 1) + 1;

    if ((desc = (struct crapool_desc*)malloc(alloc_size)) == NULL) {
        return NULL;
    }
    buf = (char*)desc + sizeof(struct crapool_desc);

    desc->next_free = buf;
    desc->free_space = alloc_size - sizeof(struct crapool_desc);
    desc->head_allocable_desc = desc;
    desc->next = NULL;
    desc->tail = desc;

#if defined(USE_PTHREAD) && defined(CRAPOOL_LOCK_ENABLED)
#if (CRAPOOL_LOCK_ENABLED == 1)
    if (mutex != NULL) {
        dbgprint("\ncrapool: Copying mutex @0x%x.\n", mutex);
        desc->mutex = mutex;
    }
    else {
        if ((desc->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t))) == NULL) {
            free(desc);
            return NULL;
        }
        dbgprint("\ncrapool: Created mutex @0x%x. \n", desc->mutex);
        pthread_mutex_init(desc->mutex, NULL);
    }
#endif
#endif
    
    dbgprint("\ncrapool: New desc @0x%x: free_space=%d, next_free=@0x%x. \n",
                desc, desc->free_space, desc->next_free);
    
    return desc;
}

static void adjust_head_allocable_desc(struct crapool_desc* const desc)
{
    struct crapool_desc* desc_avail;
    
    for (desc_avail = desc->head_allocable_desc; desc_avail != NULL;
        desc_avail = desc_avail->next)
    {
        if (desc_avail->free_space > CRAPOOL_SKIP_EXAMING_SIZE_THRESHOLD) {
            dbgprint("\ncrapool: Set head_allocable_desc pointer to @0x%x for desc @0x%x. \n",
                    desc_avail, desc);
            desc->head_allocable_desc = desc_avail;
            break;
        }
    }
}

void* crapool_alloc(struct crapool_desc* const desc, size_t size)
{
    struct crapool_desc *desc_avail;
    struct crapool_desc *curr_desc;
    char *space;

    if (size == 0) {
        return NULL;
    }

    lock_desc(desc);

    if (size < CRAPOOL_LIBC_ALLOC_THRESHOLD) {
        curr_desc = desc->head_allocable_desc;
        do {
            dbgprint("\ncrapool: Traversing desc chain through @0x%x: " \
                        "free_space=%d, next_free=@0x%x, " \
                        "head_allocable_desc=@0x%x, next=@0x%x, tail=@0x%x. \n",
                        curr_desc, curr_desc->free_space, curr_desc->next_free,
                        curr_desc->head_allocable_desc, curr_desc->next, 
                        curr_desc->tail);

            if (curr_desc->free_space >= size) {
                curr_desc->free_space -= size;
                space = curr_desc->next_free;
                curr_desc->next_free += size;
                adjust_head_allocable_desc(desc);
                unlock_desc(desc);
                dbgprint("\ncrapool: Returning allocated space @0x%x. \n", space);
                return space;
            }

            curr_desc = curr_desc->next;
        } while (curr_desc != NULL);
    }

    dbgprint("\ncrapool: Creating new descriptor. \n");
    
#if defined(USE_PTHREAD) && defined(CRAPOOL_LOCK_ENABLED)
#if (CRAPOOL_LOCK_ENABLED == 1)
    if ((desc_avail = crapool_create(size, desc->mutex)) == NULL) {
#endif
#else
    if ((desc_avail = crapool_create(size, NULL)) == NULL) {
#endif
        unlock_desc(desc);
        return NULL;
    }
    
    dbgprint("\ncrapool: Setting next pointer to @0x%x for desc @0x%x. \n",
                desc_avail, desc->tail);
    desc->tail->next = desc_avail;

    for (curr_desc = desc; curr_desc->next != NULL; 
            curr_desc = curr_desc->next) 
    {
        dbgprint("\ncrapool: Setting tail pointer to @0x%x for desc @0x%x. \n",
                    desc_avail, curr_desc);
        curr_desc->tail = desc_avail;
    }

    space = desc_avail->next_free;
    adjust_head_allocable_desc(desc);
    unlock_desc(desc);

    dbgprint("\ncrapool: Returning allocated space @0x%x. \n", space);
    return space;
}

void* crapool_calloc(struct crapool_desc* const desc, size_t count, size_t elmsize)
{
    size_t size = count * elmsize;
    void *buf;
        
    if ((buf = crapool_alloc(desc, size)) == NULL) {
        return NULL;
    }
    
    memset(buf, 0, size);
    return buf;
}
    
int crapool_destroy(struct crapool_desc* const desc)
{
    struct crapool_desc *curr;
    struct crapool_desc *next;

#if defined(USE_PTHREAD) && defined(CRAPOOL_LOCK_ENABLED)
#if (CRAPOOL_LOCK_ENABLED == 1)
    pthread_mutex_t *mtx = desc->mutex;
#endif
#endif

    lock_desc(desc);

    curr = desc;
    do {
        dbgprint("\ncrapool: Destroying desc @0x%x: free_space=%d, next_free=@0x%x," \
                    "next=@0x%x, tail=@0x%x. \n",
                    curr, curr->free_space, curr->next_free, curr->next, curr->tail);
        next = curr->next;
        free(curr);
        curr = next;
    } while(curr != NULL);

    unlock_desc(desc);

#if defined(USE_PTHREAD) && defined(CRAPOOL_LOCK_ENABLED)
#if (CRAPOOL_LOCK_ENABLED == 1)
    pthread_mutex_destroy(mtx);
#endif
#endif

    return 0;
}
