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
}

struct config {
    enum config_type type;
    void *value;
}

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

struct config *config_new();
void config_load(struct config *config, const char *path);

#endif
