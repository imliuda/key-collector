#ifndef __OSCLT_METRIC_H__
#define __OSCLT_METRIC_H__

#include <time.h>

#include "map.h"
#include "list.h"

#define METRIC_EQUAL_NAME     1
#define METRIC_EQUAL_TAGS     2
#define METRIC_EQUAL_VALUE    4
#define METRIC_EQUAL_TIME     8

enum metric_value_type {
    METRIC_VALUE_STRING_TYPE,
    METRIC_VALUE_INTEGER_TYPE,
    METRIC_VALUE_REAL_TYPE,
    METRIC_VALUE_BOOLEAN_TYPE,
    METRIC_VALUE_NULL_TYPE
};

struct metric_value {
    enum metric_value_type type;
    union {
        char *strv;
        long long intv;
        double realv;
        bool boolv;
    } data;
};

struct metric {
    char *name;
    struct map *tags;
    struct metric_value *value;
    time_t time;
};

struct metric *metric_new();
void metric_destroy(struct metric *m);
struct list *metric_parse(const char *buf);
char *metric_serialize(struct metric *m);
char *metric_list_serialize(struct list *l);
bool metric_equal(struct metric *m1, struct metric *m2, int mode);
void metric_set_name(struct metric *m, const char *name);
const char *metric_get_name(struct metric *m);
struct list *metric_tag_keys(struct metric *m);
void metric_add_tag(struct metric *m, const char *k, const char *v);
const char *metric_get_tag(struct metric *m, const char *k);
enum metric_value_type metric_value_type(struct metric_value *v);
void metric_set_string_value(struct metric *m, const char *v);
void metric_set_integer_value(struct metric *m, long long v);
void metric_set_real_value(struct metric *m, double v);
void metric_set_boolean_value(struct metric *m, bool v);
void metric_set_null_value(struct metric *m);
const char *metric_get_string_value(struct metric *m);
long long metric_get_integer_value(struct metric *m);
double  metric_get_real_value(struct metric *m);
bool metric_get_boolean_value(struct metric *m);
void metric_set_time(struct metric *m, time_t t);
time_t metric_get_time(struct metric *m);

#endif
