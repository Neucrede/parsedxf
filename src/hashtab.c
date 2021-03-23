#include <stdlib.h>
#include <string.h>
#include "hashtab.h"
#include "dbgprint.h"

static int default_keycmp(const void** const src, const void** const dest);
static size_t inflate_len(size_t p);
static int inflate(struct hashtable* const hashtab);
static int put_value(struct crapool_desc* const pool, struct hashtable_entry* const entry, 
                    void *value, int value_copy_mode, size_t value_size);

static int default_keycmp(const void** const src, const void** const dest)
{
    if (src != NULL && dest != NULL) {
        return (*(*(char** const)src) == *(*(char** const)dest)) ? 0 : 1;
    }
    
    return 1;
}

static size_t inflate_len(size_t s)
{
    switch (s) {
        case 2: return 5;
        case 5: return 11;
        case 11: return 17;
        case 17: return 37;
        case 37: return 67;
        case 67: return 131;
        case 131: return 257;
        case 257: return 521;
        case 521: return 1031;
        case 1031: return 2053;
        case 2053: return 4099;
        case 4099: return 8209;
        case 8209: return 16411;
        case 16411: return 32771;
        case 32771: return 65537;
        default: return (s << 1) + 1;
    }
}

static int inflate(struct hashtable* const hashtab)
{
    hashtab->occupied = 0;
    return 0;
}

static int put_value(struct crapool_desc* const pool, struct hashtable_entry* const entry, 
                    void* value, int value_copy_mode, size_t value_size)
{
    void *value_buf;
    
    switch (value_copy_mode) {
        case HASHTABLE_COPY_VALUE:
            memcpy(&entry->value, value, value_size);
            return 0;
        case HASHTABLE_COPY_MEMORY:
            if ((value_buf = crapool_alloc(pool, value_size)) == NULL) {
                dbgprint("hashtab: Allocate from pool failed, space required %u (bytes).\n", value_size);
                return -1;
            }
            memcpy(value_buf, value, value_size);
            entry->value.p = value_buf;
            return 0;
        default:
            return -1;
    }
}

int hashtable_create(struct hashtable* const hashtab, size_t len, float load_factor, 
                    size_t pool_size, pfn_hash_t hash_fcn, pfn_keycmp_t keycmp_fcn)
{
    if ((hashtab == NULL) || (hash_fcn == NULL)) {
        return -1;
    }
    
    if (keycmp_fcn == NULL) {
        keycmp_fcn = default_keycmp;
    }
    
    if (len == 0) {
        len = HASHTABLE_DEFAULT_LEN;
    }
    
    if ((load_factor < 0.5f) || (load_factor > 0.95f)) {
        load_factor = HASHTABLE_DEFAULT_LOAD_FACTOR;
    }
    
    if (pool_size < HASHTABLE_MIN_POOL_SIZE) {
        pool_size = HASHTABLE_DEFAULT_POOL_SIZE;
    }
    
#ifdef USE_PTHREAD
    pthread_rwlock_init(&hashtab->rwlock, NULL);
#endif
    
    if ((hashtab->table = 
        (struct hashtable_entry**)calloc(len, sizeof(struct hashtable_entry*))) == NULL) 
    {
        dbgprint("hashtab: Failed to allocate space for table entries. \n");
        return -1;
    }
    
    if ((hashtab->pool = crapool_create(pool_size, NULL)) == NULL) {
        dbgprint("hashtab: Failed to create mempool. \n");
        return -1;
    }
    
    hashtab->len = len;
    hashtab->occupied = 0;
    hashtab->inflate_threshold = (size_t)(load_factor * len);
    hashtab->hash_fcn = hash_fcn;
    hashtab->keycmp_fcn = keycmp_fcn;
    
    dbgprint("hashtab: Created hash table @0x%x, len=%u, inflate_threshold=%u, pool=@0x%x, " \
                "table=@0x%x, hash_fcn=@0x%x, keycmp_fcn=@0x%x. \n",
                hashtab, hashtab->len, hashtab->inflate_threshold, hashtab->pool,
                hashtab->table, hashtab->hash_fcn, hashtab->keycmp_fcn);

    return 0;
}

int hashtable_put(struct hashtable* const hashtab, void *key, int key_copy_mode, size_t key_size,
                    void *value, int value_copy_mode, int value_size)
{
    int retval = 0;
    
    size_t len;
    unsigned int hash;
    unsigned int slot;
    struct hashtable_entry* entry;
    void *key_buf;
    
#ifdef USE_PTHREAD
    pthread_rwlock_wrlock(hashtab->rwlock);
#endif
    
    len = hashtab->len;
    hash = hashtab->hash_fcn(key);
    slot = hash % len;
    
    for (entry = hashtab->table[slot]; entry != NULL; entry = entry->next) {
        dbgprint("hashtab: Examing entry @0x%x, hash=0x%x. \n", entry, hash);
        if (entry->hash == hash) {
            if (hashtab->keycmp_fcn((const void** const)&key, (const void ** const)(&(entry->key))) == 0) {
                dbgprint("hashtab: Key comparison succeeded. Put value into entry @0x%x, hash=0x%x. \n", entry, hash);
                retval = put_value(hashtab->pool, entry, value, value_copy_mode, value_size);
                goto hashtable_put_ret;
            }
        }
    }
    
    if (++(hashtab->occupied) >= hashtab->inflate_threshold) {
        if (inflate(hashtab) != 0) {
            goto hashtable_put_fail;
        }
        
        len = hashtab->len;
        slot = hash % len;
    }
    
    dbgprint("hashtab: Create new entry for hash table @0x%x. \n", hashtab);
    if ((entry = (struct hashtable_entry*)crapool_alloc(hashtab->pool, sizeof(struct hashtable_entry))) == NULL) {
        dbgprint("hashtab: Failed to allocate space for new entry. \n");
        goto hashtable_put_fail;
    }
    
    dbgprint("hashtab: Put key into new entry @0x%x, hash=0x%x. \n", entry, hash);
    switch (key_copy_mode) {
        case HASHTABLE_COPY_VALUE:
            memcpy(&entry->key, key, key_size);
            break;
        case HASHTABLE_COPY_MEMORY:
            if ((key_buf = crapool_alloc(hashtab->pool, key_size)) == NULL) {
                goto hashtable_put_fail;
            }
            memcpy(key_buf, key, key_size);
            entry->key.p = key_buf;
            break;
        default:
            goto hashtable_put_fail;
    }
    
    dbgprint("hashtab: Put value into new entry @0x%x. \n", entry);
    if (put_value(hashtab->pool, entry, value, value_copy_mode, value_size) != 0) {
        goto hashtable_put_fail;
    }
    
    dbgprint("hashtab: Adjust entry table. \n");
    entry->hash = hash;
    entry->next = hashtab->table[slot];
    hashtab->table[slot] = entry;
    
    goto hashtable_put_ret;
    
hashtable_put_fail:
    dbgprint("hashtab: hashtable_put() failed. \n");
    retval = -1;
hashtable_put_ret:
#ifdef USE_PTHREAD
    pthread_rwlock_unlock(hashtab->rwlock);
#endif
    return retval;
}

const void* const hashtable_get(struct hashtable* const hashtab, void *key)
{
    size_t len;
    unsigned int hash;
    unsigned int slot;
    struct hashtable_entry* entry;
    
#ifdef USE_PTHREAD
    pthread_rwlock_rdlock(hashtab->rwlock);
#endif
    
    len = hashtab->len;
    hash = hashtab->hash_fcn(key);
    slot = hash % len;
    
    for (entry = hashtab->table[slot]; entry != NULL; entry = entry->next) {
        dbgprint("hashtab: Examing entry @0x%x, hash=0x%x. \n", entry, hash);
        if (entry->hash == hash) {
            if (hashtab->keycmp_fcn((const void ** const)&key, (const void ** const)(&(entry->key))) == 0) {
#ifdef USE_PTHREAD
                pthread_rwlock_unlock(hashtab->rwlock);
#endif
                dbgprint("hashtab: Returning entry @0x%x. \n", entry);
                return (const void*)(&(entry->value));
            }
        }
    }
    
#ifdef USE_PTHREAD
    pthread_rwlock_unlock(hashtab->rwlock);
#endif
    
    return NULL;
}

int hashtable_destroy(struct hashtable* const hashtab)
{
    return 0;
}
