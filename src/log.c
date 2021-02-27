
#include <stdio.h>
#include "log.h"


void println(const char *text, ...)
{
    va_list list;

    va_start(list, text);
    vfprintf(stdout, text, list);
    fprintf(stdout, "\n");
    va_end(list);
}
