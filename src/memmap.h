#ifndef __MEMMAP_H__
#define __MEMMAP_H__

#include <sys/types.h>
#include <fcntl.h>
#include <errno.h>

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

#include <windows.h>

#ifndef O_ACCMODE
#define O_ACCMODE 0x07
#endif

#ifdef _MSC_VER
#define mode_t unsigned int
#endif

/* protect */
#define MEMMAP_READ 1
#define MEMMAP_WRITE 2
#define MEMMAP_READWRITE MEMMAP_WRITE

/* mmap flags */
#define MEMMAP_PRIVATE 1
#define MEMMAP_SHARED 2

/* sync flags */
#define MEMMAP_SYNC 1
#define MEMMAP_ASYNC 2

typedef HANDLE memmap_fd_t;

#elif defined(__linux__) || defined(__unix__) || defined(_POSIX_VERSION)

#include <sys/mman.h>
#include <unistd.h>

/* protect */
#define MEMMAP_READ PROT_READ
#define MEMMAP_WRITE PROT_WRITE
#define MEMMAP_READWRITE MEMMAP_WRITE

/* mmap flags */
#define MEMMAP_PRIVATE MAP_PRIVATE
#define MEMMAP_SHARED MAP_SHARED

/* sync flags */
#define MEMMAP_SYNC MS_SYNC
#define MEMMAP_ASYNC MS_ASYNC

typedef int memmap_fd_t;

#else
#error "Unknown platform."
#endif

#ifdef __cplusplus
extern "C" {
#endif

memmap_fd_t memmap_open(const char *filename, int flag, mode_t mode);
int memmap_close(memmap_fd_t fd);
void* memmap_map(void *address, size_t length, int protect, int flags, memmap_fd_t fd, off_t offset);
int memmap_unmap(void *addr, size_t length);
int memmap_sync(void *addr, size_t length, int flags);
size_t memmap_get_file_size(memmap_fd_t fd);

#ifdef __cplusplus
}
#endif

#endif /* __MEMMAP_H__ */

