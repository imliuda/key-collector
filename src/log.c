#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <ev.h>

#include "log.h"
#include "config.h"
#include "str.h"

#define _GNU_SOURCE

static struct queue *log_queue;
static const char *log_file;
static enum log_level log_level;
ev_io log_watcher;

static void write_log(EV_P_ ev_io *w, int revents) {
    if (revents & EV_WRITE) {
        struct log_entry *e;
        while(e = queue_front(log_queue)) {
            size_t nwrite = write(w->fd, e->text + e->nwrite, e->length - e->nwrite);
            if (nwrite == -1) {
                /* try it next time */
                break;
            } else {
                e->nwrite += nwrite;
            }
            if (e->nwrite == e->length) {
                e = queue_pop(log_queue);
                free(e->text);
                free(e);
            }
        }
    }
}

void LOG(enum log_level level, const char *fmt, va_list args) {
    if (level < log_level)
        return;
    struct log_entry *entry = malloc(sizeof(struct log_entry));
    entry->level = level;

    char time_buf[64];
    time_t t = time(NULL);
    struct tm *tm = gmtime(&t);
    size_t tlen = strftime(time_buf, 63, "%Y-%m-%d %H:%M:%S", tm);
    time_buf[tlen] = '\0';
    //free(tm);

    struct strbuf *sb = strbufnew(512);
    strbufexts(sb, time_buf); 
    if (level == LOG_DEBUG) {
        strbufexts(sb, " debug ");
    } else if (level == LOG_INFO) {
        strbufexts(sb, " info  ");
    } else if (level == LOG_WARN) {
        strbufexts(sb, " warn  ");
    } else if (level == LOG_ERROR) {
        strbufexts(sb, " error ");
    } else if (level == LOG_FATAL) {
        strbufexts(sb, " fatal ");
    }
    
    strbufextv(sb, fmt, args);

    strbufexts(sb, "\n");

    char *text = strdup(strbufstr(sb));
    strbuffree(sb);

    entry->text = text;
    entry->nwrite = 0;
    entry->length = strlen(text);

    queue_push(log_queue, entry);
}

void log_init(struct osclt *oc) {
    const char *level = config_get_string(oc->config, "log_level", "warn");

    if (strcmp(level, "debug") == 0) {
        log_level = LOG_DEBUG;
    } else if (strcmp(level, "info") == 0) {
        log_level = LOG_INFO;
    } else if (strcmp(level, "warn") == 0) {
        log_level = LOG_WARN;
    } else if (strcmp(level, "error") == 0) {
        log_level = LOG_ERROR;
    } else if (strcmp(level, "fatal") == 0) {
        log_level = LOG_FATAL;
    } else {
        fprintf(stderr, "unknown log level: \"%s\".\n", level);
        exit(1);
    }

    log_file = config_get_string(oc->config, "log_file", "/var/log/osclt/osclt.log");

    int fd = open(log_file, O_WRONLY | O_CREAT);

    if (fd == -1) {
        fprintf(stderr, "open log file failed: \"%s\".", log_file);
        exit(1);
    }

    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    ev_io_init(&log_watcher, write_log, fd, EV_WRITE);
    ev_io_start(oc->loop, &log_watcher);

    log_queue = queue_new();
}

void log_debug(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    LOG(LOG_DEBUG, fmt, ap);
    va_end(ap);
}

void log_info(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    LOG(LOG_INFO, fmt, ap);
    va_end(ap);
}

void log_warn(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    LOG(LOG_WARN, fmt, ap);
    va_end(ap);
}

void log_error(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    LOG(LOG_ERROR, fmt, ap);
    va_end(ap);
}

void log_fatal(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    LOG(LOG_FATAL, fmt, ap);
    va_end(ap);
}
