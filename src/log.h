#ifndef LOGGER_H
#define LOGGER_H

#include <stdarg.h>

/**
 * @brief 
 * 
 * @param prefix 
 * @param text 
 * @param ... 
 */
extern void log_text(const char *prefix, const char *text, va_list list);

/**
 * @brief Prints a text into stdout
 * 
 * @param text Text to be printed
 * @param ... Arguments list
 */
extern void log_notice(const char *text, ...);

/**
 * @brief 
 * 
 * @param text 
 * @param ... 
 */
extern void log_error(const char *text, ...);
#endif
