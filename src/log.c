#include <stdio.h>
#include <unistd.h>

#include "log.h"
#include "config.h"

static struct queue *log_queue;
static const char *log_file;
static enum log_level log_level;

static write_log() {

}

void log_init(struct osclt *oc) {
    log_file = config_get_string(oc->config, "log_file", "/var/log/osclt/osclt.log");
    log_level = config_get_string(oc->config, "log_level", "warn");

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
        fprintf("unknown log level: %s.\n", level);
        exit(1);
    }

    if (!access(log_file, W_OK)) {
        fprintf("open log file failed: %s.", log_file);
    }

}

void log_debug(const char *fmt, ...) {
    
}
