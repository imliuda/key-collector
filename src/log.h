#ifndef __OSCLT_LOG_H__
#define __OSCLT_LOG_H__

#include <stdarg.h>

#include "queue.h"
#include "osclt.h"

/**
 * log format:
 * 2018-08-20 14:32:46 warning command exit with status 1, reason: file not found.
 * 2018-08-20 14:32:46 warning output to server failed.
 **/

enum log_level {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
};

struct log_entry {
    enum log_level level;
    char *text;
    size_t nwrite;
    size_t length;
};

//#define log_debug(fmt, ...) LOG(LOG_DEBUG, fmt, ## __VA_ARGS__);
//#define log_info(fmt, ...) LOG(LOG_INFO, fmt, ## __VA_ARGS__);
//#define log_warn(fmt, ...) LOG(LOG_WARN, fmt, ## __VA_ARGS__);
//#define log_error(fmt, ...) LOG(LOG_ERROR, fmt, ## __VA_ARGS__);
//#define log_fatal(fmt, ...) LOG(LOG_FATAL, fmt, ## __VA_ARGS__);

void LOG(enum log_level level, const char *fmt, va_list args);
void log_init(struct osclt *oc);
void log_debug(const char *fmt, ...);
void log_info(const char *fmt, ...);
void log_warn(const char *fmt, ...);
void log_error(const char *fmt, ...);
void log_fatal(const char *fmt, ...);

#endif
