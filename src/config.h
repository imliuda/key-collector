#ifndef __OSCLT_CONFIG_H__
#define __OSCLT_CONFIG_H__

#include <stdbool.h>

#include "map.h"

#define SKIP_WHITESPACE 1
#define SKIP_COMMENT    2
#define SKIP_SEPARATOR  4

struct config_parse_buffer {
    wchar_t *buffer;
    size_t offset;
    size_t length;
};

enum config_type {
    CONFIG_OBJECT_TYPE,
    CONFIG_ARRAY_TYPE,
    CONFIG_STRING_TYPE,
    CONFIG_INTEGER_TYPE,
    CONFIG_DOUBLE_TYPE,
    CONFIG_BOOLEAN_TYPE,
    CONFIG_DURATION_TYPE
};

struct config {
    enum config_type type;
    void *value;
};

enum duration_unit {
    DURATION_NANO_SECOND,
    DURATION_MICRO_SECOND,
    DURATION_MILLI_SECOND,
    DURATION_SECOND,
    DURATION_MINUTE,
    DURATION_HOUR,
    DURATION_DAY
};

struct duration {
    enum duration_unit unit;
    long value;
};

static struct config *config_parse_object(struct config_parse_buffer *buf);
static struct config *config_parse_array(struct config_parse_buffer *buf);
static struct config *config_parse_simple(struct config_parse_buffer *buf);
static struct config *config_parse_string(struct config_parse_buffer *buf);
static struct config *config_parse_number(struct config_parse_buffer *buf);
static struct config *config_parse_boolean(struct config_parse_buffer *buf);
static struct config *config_parse_duration(struct config_parse_buffer *buf);

struct config *config_load_file(const char *path);

#endif
