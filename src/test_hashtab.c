#include <stdio.h>  /* printf(),... */
#include <string.h>
#include "hashtab.h"

unsigned int str_hash(const char *sz) {
    unsigned int hash = 0;
    
    while (*sz != '\0') {
        hash = *(sz++) + (hash << 5) - 1;
    }
    
    return 999;
}

int str_cmp(const char **psz1, const char **psz2) {
    return strcmp(*psz1, *psz2);
}

int main(int argc, char* argv[])
{
    struct hashtable hashtab;
    char *key1 = "ABCDEa123abc";
    char *key2 = "ABCDFB123abc";
    char *key3 = "jfoiewjfqewjfqeow";
    char *sz1 = "ababaabababba";
    char *sz2 = "fhfhfhhifeieifew";
    char *sz3 = "jfiqwejqioew";
    
    if (hashtable_create(&hashtab, 0, 0, 0, (pfn_hash_t)str_hash, (pfn_keycmp_t)str_cmp) != 0) {
        printf("hashtable_init() failed. \n");
        return 1;
    }
    
    hashtable_put(&hashtab, &key1, HASHTABLE_COPY_VALUE, sizeof(key1),
                    sz1, HASHTABLE_COPY_MEMORY, strlen(sz1) + 1);
    hashtable_put(&hashtab, &key2, HASHTABLE_COPY_VALUE, sizeof(key2),
                    &sz2, HASHTABLE_COPY_VALUE, sizeof(sz2));
    hashtable_put(&hashtab, &key3, HASHTABLE_COPY_VALUE, sizeof(key3),
                    sz3, HASHTABLE_COPY_MEMORY, strlen(sz3) + 1);
    
    printf("%s \n", *(char**)hashtable_get(&hashtab, key1));
    printf("%s \n", *(char**)hashtable_get(&hashtab, key2));
    printf("%s \n", *(char**)hashtable_get(&hashtab, key3));
    
    return 0;
}
