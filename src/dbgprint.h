#ifndef __DBGPRINT_H__
#define __DBGPRINT_H__

#include <stdio.h>

#if defined(DEBUG) || defined(_DEBUG) || defined(_DEBUG_) || defined(__DEBUG__)
#define dbgprint(sz, ...) fprintf(stdout, sz, ##__VA_ARGS__)
#else
#define dbgprint(sz, args...) (void)0
#endif

#endif /* __DBGPRINT_H__ */
