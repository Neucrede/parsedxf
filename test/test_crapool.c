#include <stdio.h>
#include <string.h>
#include "crapool.h"

int main()
{
    struct crapool_desc *desc;
    char *sz;
    char *sz1 = "AAAAAAAA";
    int i;
    
    if ((desc = crapool_create(4099, NULL)) == NULL) {
        printf("crapool_create() failed.\n");
    }
    
    for (i = 0; i < 100; ++i) {
        sz = crapool_alloc(desc, 8);
        memcpy(sz, sz1, 9);
    }
    
    printf("\n large block \n");
    crapool_alloc(desc, 10240000);
    
    crapool_destroy(desc);
    
    return 0;
}
