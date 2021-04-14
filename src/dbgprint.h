#ifndef __DBGPRINT_H__
#define __DBGPRINT_H__

#include <stdio.h>

#if defined(DEBUG) || defined(_DEBUG) || defined(_DEBUG_) || defined(__DEBUG__)
    #ifndef DEBUG
        #define DEBUG 1
    #endif
#endif

void enable_dbgprint(int enabled);
void enable_errprint(int enabled);
void dbgprint(const char *format, ...);
void errprint(const char *format, ...);

#endif /* __DBGPRINT_H__ */
