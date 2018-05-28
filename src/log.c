#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <time.h>
#include <errno.h>
#include <ev.h>

#include "log.h"
#include "config.h"
#include "str.h"

#define _GNU_SOURCE

static struct queue *log_queue;
static size_t log_queue_size;
static const char *log_file;
static long long log_size;
static int log_fd;
static enum log_level log_level;
static struct ev_loop *loop;
static struct ev_io write_watcher;
static struct ev_timer retry_watcher;
static struct ev_stat stat_watcher;

static void stat_log(EV_P_ ev_stat *w, int revents) {
    if (revents & EV_STAT) {
        if (!w->attr.st_nlink) { /* deleted or renamed */
            /* close old file first */
            close(log_fd);

            log_fd = open(log_file, O_RDWR | O_APPEND | O_CREAT, 0755);

            if (log_fd == -1) {
                fprintf(stderr, "rotate log file \"%s\" error: %s.\n", log_file, strerror(errno));
                exit(1);
            }
        
            int flags = fcntl(log_fd, F_GETFL, 0);
            fcntl(log_fd, F_SETFL, flags | O_NONBLOCK);

            ev_io_set(&write_watcher, log_fd, EV_NONE);
        } else if (w->attr.st_size > log_size && log_size != 0) {
            char old[strlen(log_file) + 5];
            strcpy(old, log_file);
            strcat(old, ".old");
            if (rename(log_file, old) == -1) {
                fprintf(stderr, "rename log file \"%s\" error: %s.\n", log_file, strerror(errno));
                exit(1);
            }
        }
    }
}

static void write_log_retry(EV_P_ ev_timer *w, int revents) {
    if (revents & EV_TIMER) {
        ev_feed_event(loop, &write_watcher, EV_WRITE);
    }
}

static void write_log(EV_P_ ev_io *w, int revents) {
    if (revents & EV_WRITE) {
        struct log_entry *e;
        while(e = queue_front(log_queue)) {
            size_t nwrite = write(log_fd, e->text + e->nwrite, e->length - e->nwrite);
            if (nwrite == -1) {
                if (e->nwrite && fsync(log_fd) == -1) {
                    e->nwrite = 0;
                }
                break;
            } else {
                e->nwrite += nwrite;
            }
            if (e->nwrite == e->length) {
                if (fsync(log_fd) != -1) {
                    e = queue_pop(log_queue);
                    free(e->text);
                    free(e);
                } else {
                    e->nwrite = 0;
                    break;
                }
            }
        }
    }
}

void LOG(enum log_level level, const char *fmt, va_list args) {
    if (level < log_level)
        return;

    if (queue_size(log_queue) >= log_queue_size) {
        /* just drop the data. later, may add self-monitoring static metric. */
        fprintf(stderr, "log queue full, drop new coming log...\n");
        return;
    }

    struct log_entry *entry = malloc(sizeof(struct log_entry));
    entry->level = level;

    char time_buf[64];
    time_t t = time(NULL);
    struct tm *tm = gmtime(&t);
    size_t tlen = strftime(time_buf, 63, "%Y-%m-%d %H:%M:%S", tm);
    time_buf[tlen] = '\0';

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

    ev_feed_event(loop, &write_watcher, EV_WRITE);
}

void log_init(struct osclt *oc) {
    loop = oc->loop;

    struct config *c;
 
    c = config_object_get(oc->config, "log_level");
    if (!c) {
        const char *level = "info";
    } else if (config_type(c) != CONFIG_STRING_TYPE) {
        fprintf(stderr, "invalid \"log_level\" config\n");
        exit(1);
    }

    const char *level = config_string_value(c);

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

    c = config_object_get(oc->config, "log_file");
    if (!c) {
        log_file = "/var/log/osclt/osclt.log";
    } else if (config_type(c) != CONFIG_STRING_TYPE) {
        fprintf(stderr, "invalid \"log_file\" config\n");
        exit(1);
    }
    log_file = config_string_value(c);

    c = config_object_get(oc->config, "log_size");
    if (!c) {
        /* log_size = 0 will not perform by osclt self */
        log_size = 0;
    } else if (config_type(c) != CONFIG_SIZE_TYPE) {
        fprintf(stderr, "invalid \"log_size\" config\n");
        exit(1);
    }
    log_size = config_size_value(c, CONFIG_BYTE);

    c = config_object_get(oc->config, "log_queue");
    if (!c) {
        log_queue_size = 1000;
    } else if (config_type(c) != CONFIG_INTEGER_TYPE) {
        fprintf(stderr, "invalid \"log_queue\" config\n");
        exit(1);
    }
    log_queue_size = config_integer_value(c);

    log_fd = open(log_file, O_RDWR | O_APPEND | O_CREAT, 0755);

    if (log_fd == -1) {
        fprintf(stderr, "open log file \"%s\" error: %s.\n", log_file, strerror(errno));
        exit(1);
    }

    /* O_NONBLOCK seems meaningless, fsync has been used, later may use
       separate thread loop for logging. */
    int flags = fcntl(log_fd, F_GETFL, 0);
    fcntl(log_fd, F_SETFL, flags | O_NONBLOCK);

    /* file delete or rename event, for log rotate. */
    ev_stat_init(&stat_watcher, stat_log, log_file, 0);
    ev_stat_start(loop, &stat_watcher);

    /* write log queue to file every 1 second, used for handling
       unsuccessful log write */
    ev_timer_init(&retry_watcher, write_log_retry, 0, 1);
    ev_timer_start(loop, &retry_watcher);

    /* new log comes, write it right now with ev_feed_event(). */
    ev_io_init(&write_watcher, write_log, log_fd, EV_NONE);
    ev_io_start(loop, &write_watcher);

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
