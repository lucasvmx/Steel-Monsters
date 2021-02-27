#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

/**
 * @brief Prints a text into stdout
 * 
 * @param text Text to be printed
 * @param ... Arguments list
 */
extern void println(const char *text, ...);

#endif
