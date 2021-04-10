#include <stdio.h>
#include <stdarg.h>
#include "dbgprint.h"

#if defined(_MSC_VER) && _MSC_VER < 1400

#ifndef NO_DBGPRINT
void dbgprint(const char *format, ...)
{
    va_list arg_list;

    va_start(arg_list, format);
    vfprintf(stdout, format, arg_list);
    va_end(arg_list);
}
#else
void dbgprint(const char *format, ...)
{
    (void)format;
}
#endif

#ifndef NO_ERRPRINT
void errprint(const char *format, ...)
{
    va_list arg_list;

    va_start(arg_list, format);
    vfprintf(stderr, format, arg_list);
    va_end(arg_list);
}
#else
void errprint(const char *format, ...)
{
    (void)format;
}
#endif

#endif /* defined(_MSC_VER) && _MSC_VER < 1400 */
