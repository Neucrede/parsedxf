#ifndef __DBGPRINT_H__
#define __DBGPRINT_H__

#include <stdio.h>

#ifdef NO_DBGPRINT
    #define dbgprint(sz, ...) (void)0
#elif defined(DEBUG) || defined(_DEBUG) || defined(_DEBUG_) || defined(__DEBUG__)
    #ifndef DEBUG
        #define DEBUG 1
    #endif

    #define dbgprint(sz, ...) fprintf(stdout, sz, ##__VA_ARGS__)
#else
    #define dbgprint(...)
#endif

#ifdef NO_ERRPRINT
    #define errprint(sz, ...) (void)0
#else
    #define errprint(sz, ...) fprintf(stderr, sz, ##__VA_ARGS__)
#endif

#endif /* __DBGPRINT_H__ */
