
#include <stdio.h>
#include <stdarg.h>
#include "log.h"

namespace rs_frame {

void log_debug(const char *fmt, ...)
{
    static FILE *fp(fopen("./data/log", "a"));
    va_list args;

    if (fp == NULL) {
        return;
    }
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    fflush(fp);
    va_end(args);
}

} // end of namespace rs_frame

