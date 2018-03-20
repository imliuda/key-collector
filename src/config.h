#ifndef __OSCLT_CONFIG_H__
#define __OSCLT_CONFIG_H__

#include "map.h"

enum config_type {
    CONFIG_OBJECT_TYPE,
    CONFIG_ARRAY_TYPE,
    CONFIG_STRING_TYPE,
    CONFIG_INTEGER_TYPE,
    CONFIG_FLOAT_TYPE,
    CONFIG_BOOLEAN_TYPE,
    CONFIG_DURATION_TYPE
};

struct config {
    enum config_type type;
    void *value;
};

struct duration {
    struct timespec time;
};

enum config_parse_state {
    CONFIG_PARSE_KEY,
    CONFIG_PARSE_VALUE,
    CONFIG_PARSE_OBJECT,
    CONFIG_PARSE_LIST,
    CONFIG_PARSE_COMMENT,
    CONFIG_PARSE_NONE
};

static struct config *config_parse_object(wchar_t buf[], size_t *cursor, size_t buflen);
static struct config *config_parse_array(wchar_t buf[], size_t *cursor, size_t buflen);
static struct config *config_parse_simple(wchar_t buf[], size_t *cursor, size_t buflen);

struct config *config_load(const char *path);

#endif
