#ifndef __OSCLT_CONFIG_H__
#define __OSCLT_CONFIG_H__

#include <stdbool.h>

#include "map.h"

#define WHITESPACE  1
#define COMMENT     2
#define SEPARATOR   4

enum config_type {
    CONFIG_OBJECT_TYPE,
    CONFIG_ARRAY_TYPE,
    CONFIG_STRING_TYPE,
    CONFIG_INTEGER_TYPE,
    CONFIG_FLOAT_TYPE,
    CONFIG_BOOLEAN_TYPE,
    CONFIG_DURATION_TYPE
};

struct config_parse_buffer {
    wchar_t buffer[];
    size_t offset;
    size_t length;
}

struct config {
    enum config_type type;
    void *value;
};

struct duration {
    struct timespec time;
};

static struct config *config_parse_object(struct config_parse_buffer *buf);
static struct config *config_parse_array(struct config_parse_buffer *buf);
static struct config *config_parse_simple(struct config_parse_buffer *buf);

struct config *config_load_file(const char *path);

#endif
