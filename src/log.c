#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>

#include "log.h"
#include "config.h"
#include "str.h"

static struct queue *log_queue;
static const char *log_file;
static uv_file file;
static enum log_level log_level;
static uv_loop *loop;

static void write_log(enum log_level level, const char *fmt, ...) {
    if (level < log_level)
        return;
    struct log_entry *entry = malloc(sizeof(struct log_entry));
    entry->level = level;

    char time_buf[64];
    time_t t = time(NULL);
    struct tm *tm = gmtime(&t);
    size_t tlen = strftime(time_buf, 63, "%Y-%m-%d %H:%M%S", tm);
    time_buf[tlen] = '\0';
    //free(tm);

    struct strbuf *sb = strbufnew(512);
    strbufexts(sb, time_buf); 
    if (level == LOG_DEBUG) {
        strbufexts(sb, " debug   ");
    } else if (level == LOG_INFO) {
        strbufexts(sb, " info    ");
    } else if (level == LOG_INFO) {
        strbufexts(sb, " warning ");
    } else if (level == LOG_INFO) {
        strbufexts(sb, " error   ");
    } else if (level == LOG_INFO) {
        strbufexts(sb, " fatal   ");
    }
    
    va_list args;
    va_start(args, fmt);
    strbufextf(sb, fmt, args);
    va_end(args);

    char *text = strbufstr(sb);
    strbuffree(sb);

    entry->text = text;
    entry->nwrite = 0;
    entry->length = strlen(text) - 1;

    queue_push(log_queue, entry);

    struct log_entry *entry = queue_front(log_queue);
    uv_fs_write(loop, &file, );
    uv_timer_t timer_req;
    uv_timer_init(oc->loop, &timer_req);
    uv_timer_start(&timer_req, log_writer, 0, 100);
}

void log_init(struct osclt *oc) {
    loop = oc->loop;

    const char *level = config_get_string(oc->config, "log_level", "warning");

    if (strcmp(level, "debug") == 0) {
        log_level = LOG_DEBUG;
    } else if (strcmp(level, "info") == 0) {
        log_level = LOG_INFO;
    } else if (strcmp(level, "warn") == 0) {
        log_level = LOG_WARNING;
    } else if (strcmp(level, "error") == 0) {
        log_level = LOG_ERROR;
    } else if (strcmp(level, "fatal") == 0) {
        log_level = LOG_FATAL;
    } else {
        fprintf(stderr, "unknown log level: %s.\n", level);
        exit(1);
    }

    log_file = config_get_string(oc->config, "log_file", "/var/log/osclt/osclt.log");

    if (!access(log_file, W_OK)) {
        fprintf(stderr, "open log file failed: %s.", log_file);
    }

    uv_fs_open(loop, &file, log_file, O_WRONLY, 0, NULL);

    log_queue = queue_new();
}

void log_debug(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    write_log(LOG_DEBUG, fmt, args);
    va_end(args);
}

void log_info(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    write_log(LOG_INFO, fmt, args);
    va_end(args);
}

void log_warning(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    write_log(LOG_WARNING, fmt, args);
    va_end(args);
}

void log_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    write_log(LOG_ERROR, fmt, args);
    va_end(args);
}

void log_fatal(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    write_log(LOG_FATAL, fmt, args);
    va_end(args);
}

