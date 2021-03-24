#include "memmap.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

memmap_fd_t memmap_open(const char *filename, int flag, mode_t mode)
{
    HANDLE h;
    DWORD access = GENERIC_READ;
    int acc_mode = flag & O_ACCMODE;
    DWORD dispos = OPEN_EXISTING;

    access |= (acc_mode & O_RDONLY) ? GENERIC_READ : 0;
    access |= (acc_mode & O_WRONLY || acc_mode & O_RDWR) ? GENERIC_WRITE : 0;

    if (flag & O_CREAT) {
        if (flag & O_EXCL) {
            dispos = CREATE_NEW;
        }
        else {
            dispos = OPEN_ALWAYS;
        }
    } 
    else if (flag & O_TRUNC) {
        dispos = TRUNCATE_EXISTING;
    }

    h = CreateFile(filename, access, FILE_SHARE_READ, NULL, dispos, 
            FILE_ATTRIBUTE_NORMAL, NULL);
    if (h == INVALID_HANDLE_VALUE) {
        return (memmap_fd_t)(-1);
    }

    (void)mode;

    return (memmap_fd_t)h;
}

int memmap_close(memmap_fd_t fd)
{
    return CloseHandle((HANDLE)fd) != 0 ? 0 : -1;
}

void* memmap_map(void *address, size_t length, int protect, int flags, memmap_fd_t fd, off_t offset)
{
    HANDLE mapobj;
    void *map_addr;
    DWORD flprotect = PAGE_READONLY;
    DWORD access = FILE_MAP_READ;

    if (flags & MEMMAP_PRIVATE) {
        if (protect & MEMMAP_READWRITE) {
            flprotect = PAGE_WRITECOPY;
            access = FILE_MAP_COPY;
        }
    } 
    else if (flags & MEMMAP_SHARED) {
        if (protect & MEMMAP_READWRITE) {
            flprotect = PAGE_READWRITE;
            access = FILE_MAP_ALL_ACCESS;
        }
    }

    mapobj = CreateFileMapping((HANDLE)fd, NULL, flprotect, 0, 0, NULL);
    if (mapobj == NULL) {
        return NULL;
    }

    map_addr = MapViewOfFileEx(
            mapobj, 
            access, 
            0, 
            (DWORD)(offset & 0xFFFFFFFF), 
            (DWORD)length, 
            address);

    return map_addr;
}

int memmap_unmap(void *addr, size_t length)
{
    (void)length;

    return UnmapViewOfFile(addr) != 0 ? 0 : -1;
}

int memmap_sync(void *addr, size_t length, int flags)
{
    (void)length;
    (void)flags;

    return FlushViewOfFile(addr, 0) != 0 ? 0 : -1;
}

size_t memmap_get_file_size(memmap_fd_t fd)
{
    return (size_t)GetFileSize(fd, NULL);
}

#elif defined(__linux__) || defined(__unix__) || defined(_POSIX_VERSION)

#include <sys/stat.h>

memmap_fd_t memmap_open(const char *filename, int flag, mode_t mode)
{
    return open(filename, flag, mode);
}

int memmap_close(memmap_fd_t fd)
{
    return close(fd);
}

void* memmap_map(void *address, size_t length, int protect, int flags, memmap_fd_t fd, off_t offset)
{
    size_t file_len;

    if (length > 0) {
        return mmap(address, length, protect, flags, fd, offset);
    }
    else {
        file_len = memmap_get_file_size(fd);
        return mmap(address, file_len, protect, flags, fd, offset);
    }
}

int memmap_unmap(void *addr, size_t length)
{
    return munmap(addr, length);
}

int memmap_sync(void *addr, size_t length, int flags)
{
    return msync(addr, length, flags);
}

size_t memmap_get_file_size(memmap_fd_t fd)
{
    struct stat st;
        
    if (fstat(fd, &st) != 0) {
        return 0;
    }

    return (size_t)st.st_size;
}

#endif
