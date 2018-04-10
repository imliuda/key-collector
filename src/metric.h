#ifndef __OSCLT_METRIC_H__
#define __OSCLT_METRIC_H__

#include <time.h>

#include "map.h"

#define METRIC_EQUAL_NAME     1
#define METRIC_EQUAL_TAGS     2
#define METRIC_EQUAL_FIELDS   4
#define METRIC_EQUAL_TIME     8

enum field_value_type { 
    FIELD_STRING_TYPE,
    DIELD_INTEGER_TYPE,
    FIELD_FLOAT_TYPE,
    FIELD_BOOLEAN_TYPE
};

struct field_value {
    enum field_value_type type;
    union {
        double dbl_v;
        int64_t int_v;
        char *str_v;
        bool bool_v;
    } value;
};

struct metric {
    char *name;
    struct map *tags;
    struct map *fields;
    time_t time;
};

struct metric *metric_new();
void metric_destroy(struct metric *metric);
struct list *metric_parse(const char *buf);
const char *metric_serialize(struct list *metrics);
bool metric_equal(struct metric *metric1, struct metric *metric2, int mode);
const char *metric_get_name(struct metric *metric);
void metric_set_name(struct metric *metric, char *name);
struct list * metric_tag_keys(struct metric *metric);
void metric_add_tag(struct metric *metric, const char *tag, const char *value);
char *metric_get_tag(struct metric *metric, const char *tag);
bool metric_remove_tag(struct metric *metric, const char *tag);
struct list *metric_field_keys(struct metric *metric);
void metric_add_field(struct metric *metric, const char *field, struct field_value *value);
struct field_value *metric_get_field(struct metric *metric, const char *field);
bool metric_remove_field(struct metric *metric, const char *key);
time_t metric_get_time(struct metric *metric);
void metric_set_time(struct metric *metric, time_t time);

#endif
