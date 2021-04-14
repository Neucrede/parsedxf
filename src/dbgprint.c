#include <stdio.h>
#include <stdarg.h>
#include "dbgprint.h"

#if defined(DEBUG) && !defined(NO_DBGPRINT)
static int dbgprint_enabled = 1;
#else
static int dbgprint_enabled = 0;
#endif

#if defined(NO_ERRPRINT)
static int errprint_enabled = 0;
#else
static int errprint_enabled = 1;
#endif

void enable_dbgprint(int enabled)
{
    dbgprint_enabled = enabled;
}

void enable_errprint(int enabled)
{
    errprint_enabled = enabled;
}

void dbgprint(const char *format, ...)
{
    va_list arg_list;
    
    if (dbgprint_enabled != 0) {
        va_start(arg_list, format);
        vfprintf(stdout, format, arg_list);
        va_end(arg_list);
    }
}

void errprint(const char *format, ...)
{
    va_list arg_list;

    if (errprint_enabled != 0) {
        va_start(arg_list, format);
        vfprintf(stderr, format, arg_list);
        va_end(arg_list);
    }       
}

