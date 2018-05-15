#ifndef __OSCLT_CONFIG_H__
#define __OSCLT_CONFIG_H__

#include <stdbool.h>

#include "map.h"

#define SKIP_WHITESPACE 1
#define SKIP_COMMENT    2
#define SKIP_SEPARATOR  4

struct config_parser {
    wchar_t *buffer;
    size_t offset;
    size_t length;
};

enum config_error_code {
    
};

struct config_error {
    enum config_error_code code;
    const char *text;
    size_t line;
    size_t column;
    size_t position;
};

enum config_type {
    CONFIG_OBJECT_TYPE,
    CONFIG_ARRAY_TYPE,
    CONFIG_STRING_TYPE,
    CONFIG_INTEGER_TYPE,
    CONFIG_DOUBLE_TYPE,
    CONFIG_BOOLEAN_TYPE,
    CONFIG_DURATION_TYPE,
    CONFIG_SIZE_TYPE
};

struct config {
    enum config_type type;
    void *value;
};

enum config_duration_unit {
    CONFIG_NANO_SECOND,
    CONFIG_MICRO_SECOND,
    CONFIG_MILLI_SECOND,
    CONFIG_SECOND,
    CONFIG_MINUTE,
    CONFIG_HOUR,
    CONFIG_DAY
};

enum config_size_unit {
    CONFIG_BYTE,
    CONFIG_KILO_BYTE,
    CONFIG_MEGA_BYTE,
    CONFIG_GIGA_BYTE,
    CONFIG_TERA_BYTE,
    CONFIG_PETA_BYTE,
    CONFIG_EXA_BYTE,
    CONFIG_ZETTA_BYTE,
    CONFIG_YOTTA_BYTE
};

struct config_duration {
    enum config_unit unit;
    long long value;
};

struct config_size {
    enum config_unit unit;
    long long value;
};

extern const char **config_error_text;

struct config *config_load(const char *path, char **errmsg);
void config_dumps(struct config *config);
void config_destroy(struct config *config);
size_t config_array_size(struct config *config);
struct config *config_array_get(struct config *config, size_t index);
struct list *config_object_keys(struct config *config);
struct config *config_object_get(struct config *config, ...);
const char *config_string_value(struct config *config);
long long config_integer_value(struct config *config);
double config_double_value(struct config *config);
bool config_boolean_value(struct config *config);
long long config_duration_value(struct config *config, enum config_duration_unit unit);
long long config_size_value(struct config *config, enum config_size_unit unit);

#endif
