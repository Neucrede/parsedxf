#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "crapool.h"

#include "dbgprint.h"

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

static void adjust_head_allocable_desc(struct crapool_desc* const desc)
{
    struct crapool_desc* desc_avail;
    
    for (desc_avail = desc->head_allocable_desc; desc_avail != NULL;
        desc_avail = desc_avail->next)
    {
        if (desc_avail->free_space > CRAPOOL_SKIP_EXAMING_SIZE_THRESHOLD) {
            dbgprint("crapool: adjust_head_allocable_desc(): " \
                    "Set head_allocable_desc pointer to @0x%lx for desc @0x%lx. \n",
                    (unsigned long)desc_avail, (unsigned long)desc);
            desc->head_allocable_desc = desc_avail;
            break;
        }
    }
}

struct crapool_desc* crapool_create(size_t size, void* const mutex)
{
#ifdef CRAPOOL_DISABLED
    (void)size;
    (void)mutex;
    return (struct crapool_desc*)(-1);
#else
    size_t alloc_size;
    struct crapool_desc *desc;
    char *buf;

    if (size < CRAPOOL_SIZE_MIN) {
        size = CRAPOOL_SIZE_MIN;
    }
    
    alloc_size = 
        ((size + sizeof(struct crapool_desc)) | (CRAPOOL_SIZE_MULTIPLE - 1)) + 1;

    if ((desc = (struct crapool_desc*)malloc(alloc_size)) == NULL) {
        errprint("crapool: crapool_create(): Failed to allocate memory. \n");
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
        dbgprint("crapool: crapool_create(): Copying mutex @0x%lx.\n", 
            (unsigned long)mutex);
        desc->mutex = mutex;
    }
    else {
        if ((desc->mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t))) == NULL) {
            free(desc);
            return NULL;
        }
        dbgprint("crapool: crapool_create(): Created mutex @0x%lx. \n", 
            (unsigned long)(desc->mutex));
        pthread_mutex_init(desc->mutex, NULL);
    }
#endif
#endif
    
    dbgprint("crapool: crapool_create(): New desc @0x%lx: free_space=%zu, " \
                "next_free=@0x%lx. \n",
                (unsigned long)desc, desc->free_space, (unsigned long)(desc->next_free));
    
    return desc;
#endif
}

void* crapool_alloc(struct crapool_desc* const desc, size_t size)
{
#ifdef CRAPOOL_DISABLED
    (void)desc;
    return malloc(size);
#else
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
            dbgprint("crapool: crapool_alloc(): Traversing desc chain through @0x%lx: " \
                        "free_space=%zu, next_free=@0x%lx, " \
                        "head_allocable_desc=@0x%lx, next=@0x%lx, tail=@0x%lx. \n",
                        (unsigned long)curr_desc, curr_desc->free_space, 
                        (unsigned long)(curr_desc->next_free),
                        (unsigned long)(curr_desc->head_allocable_desc), 
                        (unsigned long)(curr_desc->next), 
                        (unsigned long)(curr_desc->tail));

            if (curr_desc->free_space >= size) {
                curr_desc->free_space -= size;
                space = curr_desc->next_free;
                curr_desc->next_free += size;
                adjust_head_allocable_desc(desc);
                unlock_desc(desc);
                dbgprint("crapool: crapool_alloc(): Returning allocated space @0x%lx. \n", (unsigned long)space);
                return space;
            }

            curr_desc = curr_desc->next;
        } while (curr_desc != NULL);
    }

    dbgprint("crapool: crapool_alloc(): Creating new descriptor. \n");
    
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
    
    dbgprint("crapool: crapool_alloc(): Setting next pointer to @0x%lx for desc @0x%lx. \n",
                (unsigned long)desc_avail, (unsigned long)(desc->tail));
    desc->tail->next = desc_avail;

    for (curr_desc = desc; curr_desc->next != NULL; 
            curr_desc = curr_desc->next) 
    {
        dbgprint("crapool: crapool_alloc(): Setting tail pointer to @0x%lx for desc @0x%lx. \n",
                    (unsigned long)desc_avail, (unsigned long)curr_desc);
        curr_desc->tail = desc_avail;
    }

    desc_avail->free_space -= size;
    space = desc_avail->next_free;
    desc_avail->next_free += size;
    adjust_head_allocable_desc(desc);
    unlock_desc(desc);

    dbgprint("crapool: crapool_alloc(): Returning allocated space @0x%lx. \n", 
        (unsigned long)space);
    return space;
#endif
}

void* crapool_calloc(struct crapool_desc* const desc, size_t count, size_t elmsize)
{
#ifdef CRAPOOL_DISABLED
    (void)desc;
    return calloc(count, elmsize);
#else
    size_t size = count * elmsize;
    void *buf;
        
    if ((buf = crapool_alloc(desc, size)) == NULL) {
        errprint("crapool: crapool_calloc(): crapool_alloc() failed. \n");
        return NULL;
    }
    
    memset(buf, 0, size);
    return buf;
#endif
}
    
int crapool_destroy(struct crapool_desc* const desc)
{
#ifdef CRAPOOL_DISABLED
    (void)desc;
    return 0;
#else
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
        dbgprint("crapool: crapool_destroy(): Destroying desc @0x%lx: " \
                    "free_space=%zu, next_free=@0x%lx, " \
                    "next=@0x%lx, tail=@0x%lx. \n",
                    (unsigned long)curr, curr->free_space,
                    (unsigned long)(curr->next_free), 
                    (unsigned long)(curr->next), 
                    (unsigned long)(curr->tail));
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
#endif
}
