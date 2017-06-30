#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <limits.h>
#include <libgen.h>
#include "log.h"
#include "config.h"

static FILE *fp = NULL;
static char *log_file;
static int log_level;

bool mkdirp(const char *path, mode_t mode) {
    char p[PATH_MAX] = "";
    char *first, *second, *tmp;

    second = tmp = strdup(path);
    while ((first = strsep(&second, "/")) != NULL) {
        strcat(p, first);
        strcat(p, "/");
        if (mkdir(p, mode) == -1 && errno != EEXIST) {
            return false;
        }
    }
    free(tmp);
    return true;
}

void log_init() {
    char *level = config_get_string("LogLevel", "warn");
    if (strcasecmp(level, "debug") == 0) {
        log_level = LOG_DEBUG;
    } else if (strcasecmp(level, "info") == 0) {
        log_level = LOG_INFO;
    } else if (strcasecmp(level, "warn") == 0) {
        log_level = LOG_INFO;
    } else if (strcasecmp(level, "error") == 0) {
        log_level = LOG_INFO;
    } else if (strcasecmp(level, "fatal") == 0) {
        log_level = LOG_INFO;
    }

    log_file = config_get_string("LogFile", "/var/log/osclt/osclt.log");
    char *tmp = strdup(log_file);
    char *log_dir = dirname(tmp);

    if (!mkdirp(log_dir, 0777)) {
        printf("failed to create log dir: %s, %s.\n", log_dir, strerror(errno));
        free(tmp);
        exit(1);
    }
    free(tmp);

    fp = fopen(log_file, "a+");
    if (fp == NULL) {
        printf("failed to open log file: %s, %s\n", log_file, strerror(errno));
        exit(1);
    }
}

void clt_log(LOG_LEVEL level, const char *fmt, ...) {
    
}
