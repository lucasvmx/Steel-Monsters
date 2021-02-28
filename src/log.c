
#include <stdio.h>
#include "log.h"

void log_text(const char *prefix, const char *text, va_list list)
{
    printf("%s ", prefix);
    vfprintf(stdout, text, list);
    fprintf(stdout, "\n");
}

void log_notice(const char *text, ...)
{
    va_list list;

    va_start(list, text);
    log_text("[INFO] ", text, list);
    va_end(list);
}

void log_error(const char *text, ...)
{
    va_list list;

    va_start(list, text);
    log_text("[ERROR] ", text, list);
    va_end(list);
}
