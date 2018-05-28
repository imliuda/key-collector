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
    METRIC_VALUE_NULL_TYPE,
};

struct metric_value {
    enum metric_value_type type;
    void *value;
};

struct metric {
    char *name;
    struct map *tags;
    struct metric_value *value;
    time_t time; /* ((time_t) -1) represent no time */
};

enum metric_error_code {
    METRIC_ERROR_INVALID_TAG_KEY
};

struct metric_error {
    enum metric_error_code code;
    const char *text;
    size_t line;
    size_t column;
    size_t position;
};

struct metric_parser {
    wchar_t *buffer;
    size_t offset;
    size_t length;
};

extern const char **metric_error_text;

struct metric *metric_new();
void metric_destroy(struct metric *m);
void metric_destroy_list(struct list *ms);
bool metric_validate(struct metric *m);
struct list *metric_parse(const char *buf, struct metric_error *e);
char *metric_serialize(struct metric *m);
char *metric_serialize_list(struct list *ms);
bool metric_equal(struct metric *m1, struct metric *m2, int mode);
void metric_set_name(struct metric *m, const char *name);
const char *metric_get_name(struct metric *m);
struct list *metric_tag_keys(struct metric *m);
void metric_add_tag(struct metric *m, const char *k, const char *v);
const char *metric_get_tag(struct metric *m, const char *k);
enum metric_value_type metric_value_type(struct metric *m);
void metric_set_string_value(struct metric *m, const char *v);
void metric_set_integer_value(struct metric *m, long long v);
void metric_set_real_value(struct metric *m, double v);
void metric_set_boolean_value(struct metric *m, bool v);
void metric_set_null_value(struct metric *m);
const char *metric_get_string_value(struct metric *m);
long long metric_get_integer_value(struct metric *m);
double metric_get_real_value(struct metric *m);
bool metric_get_boolean_value(struct metric *m);
void metric_set_time(struct metric *m, time_t t);
time_t metric_get_time(struct metric *m);

#endif
