#ifndef __HASHTAB_H__
#define __HASHTAB_H__

#include "crapool.h"

#ifdef USE_PTHREAD
    #include <pthread.h>
#endif

#define HASHTABLE_DEFAULT_LOAD_FACTOR 0.75f
#define HASHTABLE_DEFAULT_LEN 67
#define HASHTABLE_MIN_POOL_SIZE 4096
#define HASHTABLE_DEFAULT_POOL_SIZE 4096
#define HASHTABLE_COPY_VALUE 0
#define HASHTABLE_COPY_MEMORY 1

typedef unsigned int (*pfn_hash_t)(const void* const);
typedef int (*pfn_keycmp_t)(const void** const, const void** const);

struct hashtable_entry;
struct hashtable_entry {
    unsigned int hash;
    struct hashtable_entry *next;
    union {
        char c;
        int i;
        long l;
        long long ll;
        float f;
        double d;
        char *p;
    } key;
    union {
        char c;
        int i;
        long l;
        long long ll;
        float f;
        double d;
        char *p;
    } value;
};

struct hashtable {
    size_t len;
    size_t occupied;
    size_t inflate_threshold;
    struct crapool_desc *pool;
    struct hashtable_entry **table;
    pfn_hash_t hash_fcn;
    pfn_keycmp_t keycmp_fcn;
    
#if defined(USE_PTHREAD) && defined(CRAPOOL_LOCK_ENABLED)
#if (CRAPOOL_LOCK_ENABLED == 1)
    pthread_rwlock_t rwlock;
#endif
#endif
};

#ifdef __cplusplus
extern "C" {
#endif
    
int hashtable_create(struct hashtable* const hashtab, size_t len, float load_factor, 
                    size_t pool_size, pfn_hash_t hash_fcn, pfn_keycmp_t keycmp_fcn);
int hashtable_put(struct hashtable* const hashtab, void *key, int key_copy_mode, size_t key_size,
                    void *value, int value_copy_mode, size_t value_size);
void* hashtable_get(struct hashtable* const hashtab, void *key);
int hashtable_destroy(struct hashtable* const hashtab);

#ifdef __cplusplus
}
#endif

#endif /* __HASHTAB_H__ */
