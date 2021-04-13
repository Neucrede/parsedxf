#ifndef __HASHTAB_H__
#define __HASHTAB_H__

#ifdef _MSC_VER
#define __INT64_T__ __int64
#define __UINT64_T__ unsigned __int64
#else
#include <stdint.h>
#define __INT64_T__ int64_t
#define __UINT64_T__ uint64_t
#endif

#ifdef USE_PTHREAD
    #include <pthread.h>
#endif

#include "crapool.h"

#define HASHTABLE_DEFAULT_LOAD_FACTOR 0.75
#define HASHTABLE_DEFAULT_LEN 67
#define HASHTABLE_MIN_POOL_SIZE 4096
#define HASHTABLE_DEFAULT_POOL_SIZE 4096
#define HASHTABLE_COPY_VALUE 0
#define HASHTABLE_COPY_MEMORY 1

typedef unsigned int (*pfn_hash_t)(const void* const);
typedef int (*pfn_keycmp_t)(const void** const, const void** const);
typedef int (*pfn_element_destroy_hook_t)(void** const);

struct hashtable_entry;
struct hashtable_entry {
    unsigned int hash;
    struct hashtable_entry *next;
    union {
        char c;
        int i;
        long l;
        __INT64_T__ ll;
        float f;
        double d;
        char *p;
    } key;
    union {
        char c;
        int i;
        long l;
        __INT64_T__ ll;
        float f;
        double d;
        char *p;
    } value;
};

struct hashtable {
    size_t len;
    size_t occupied;
    size_t inflate_threshold;
    double load_factor;
    struct crapool_desc *pool;
    struct hashtable_entry **table;
    int key_copy_mode;
    int value_copy_mode;
    pfn_hash_t hash_fcn;
    pfn_keycmp_t keycmp_fcn;
    pfn_element_destroy_hook_t elem_destroy_fcn;
    
#if defined(USE_PTHREAD) && defined(CRAPOOL_LOCK_ENABLED)
#if (CRAPOOL_LOCK_ENABLED == 1)
    pthread_rwlock_t rwlock;
#endif
#endif
};

#ifdef __cplusplus
extern "C" {
#endif
    
int hashtable_create(struct hashtable* const hashtab, size_t len, double load_factor, 
                    size_t pool_size, int key_copy_mode, int value_copy_mode, 
                    pfn_hash_t hash_fcn, pfn_keycmp_t keycmp_fcn,
                    pfn_element_destroy_hook_t elem_destroy_fcn);
int hashtable_put(struct hashtable* const hashtab, void *key, size_t key_size, 
                  void *value, size_t value_size);
void* hashtable_get(struct hashtable* const hashtab, void *key);
int hashtable_destroy(struct hashtable* const hashtab);

#ifdef __cplusplus
}
#endif

#endif /* __HASHTAB_H__ */
