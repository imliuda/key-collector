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

struct config *config_load(const char *path);
void config_dumps(struct config *config);
struct config *config_get_object(struct config *config, const char *key);
struct config *config_get_array(struct config *config, const char *key);
const char *config_get_string(struct config *config, const char *key, const char *def);
long long config_get_integer(struct config *config, const char *key, long long def);
double config_get_double(struct config *config, const char *key, double def);
bool config_get_boolean(struct config *config, const char *key, bool def);
struct duration config_get_duration(struct config *config, const char *key, long long value, enum duration_unit unit);

#endif
