#ifndef __OSCLT_LOG_H__
#define __OSCLT_LOG_H__

#include "queue.h"
#include "osclt.h"

struct log_entry {
    time_t time;
    enum log_level level;
    char *text;
    size_t nwrite;
    size_t length;
};

static struct queue log_queue = NULL;

void log_init(struct osclt *oc);
/**
 * format:
 * 2018-08-20 14:32:46 warning command exit with status 1, reason: file not found.
 * 2018-08-20 14:32:46 warning output to server failed.
 **/
void log_debug(const char *fmt, ...);
void log_info(const char *text, ...);
void log_warning(const char *text, ...);
void log_error(const char *text, ...);
void log_fatal(const char *text, ...);

#endif
