#include <stdio.h>
#include <string.h>
#include "memmap.h"


#ifdef __linux__
int GetLastError()
{
    return errno;
}
#endif

int main()
{
    memmap_fd_t fd;
    char *sz;
    char *sz2 = "AHDHDHDHHHEI";

    fd = memmap_open("./1.txt", O_RDWR, 00777);
    printf("memmap_open: %d\n", GetLastError());

    sz = (char*)memmap_map(NULL, 16, MEMMAP_READWRITE, MEMMAP_SHARED,
            fd, 0);
    printf("%s\n", sz);
    sz[0] = 'D';
    printf("%s\n", sz);

    printf("memmap_map: %d\n", GetLastError());
    if (sz == NULL) {
        printf("sz == NULL\n");
        return 1;
    }

    memmap_sync(sz, 0, MEMMAP_SYNC);
    printf("memmap_sync: %d\n", GetLastError());

    memmap_unmap(sz, 16);
    printf("memmap_unmap: %d\n", GetLastError());

    memmap_close(fd);
    printf("memmap_close: %d\n", GetLastError());
    return 0;
}


