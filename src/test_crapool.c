#include <stdio.h>
#include <string.h>
#include "crapool.h"

int main()
{
    struct crapool_desc *desc;
    char *sz1, *sz2, *sz3, *sz4, *sz5;
    
    if ((desc = crapool_create(4099, NULL)) == NULL) {
        printf("crapool_create() failed.\n");
    }
    
    printf("\n sz1 \n");
    sz1 = crapool_alloc(desc, 1024);
    
    printf("\n sz2 \n");
    sz2 = crapool_alloc(desc, 1024);
    
    printf("\n sz3 \n");
    sz3 = crapool_alloc(desc, 2048);
    
    printf("\n sz4 \n");
    sz4 = crapool_alloc(desc, 2048);
    
    printf("\n sz5 \n");
    sz5 = crapool_alloc(desc, 32);

    printf("\n large block \n");
    crapool_alloc(desc, 10240000);
    
    strcpy(sz1, "FFFFFFFFFFFFFFFFFFFFFFFFFF");
    strcpy(sz2, "GGGGGGGGGGGGGGGGGGGGGGGGGG");
    strcpy(sz3, "HHHHHHHHHHHHHHHHHHHHHHHHHH");
    strcpy(sz4, "IIIIIIIIIIIIIIIIIIIIIIIIII");
    strcpy(sz5, "JJJJJJJJJJJJJJJJJJJJJJJJJJ");
    
    crapool_destroy(desc);
    
    return 0;
}
