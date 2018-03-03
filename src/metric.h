#ifndef __OSCLT_METRIC_H__
#define __OSCLT_METRIC_H__

#include <time.h>

#include "map.h"

#define EQ_NAME     1
#define EQ_TAGS     2
#define EQ_FIELDS   4
#define EQ_TIME     8

enum metric_vtype { FLOAT, INTEGER, STRING, BOOLEAN};
enum metric_precision {NSEC, USEC, MSEC, SECOND, MINUTE, HOUR};

struct metric_value {
    enum metric_vtype type;
    union {
        double fv;
        int64_t iv;
        char *sv;
        bool bv;
    } value;
}

struct metric_time {
    struct timespec time;
    enum precision precision;
}

struct metric {
    char *name;
    struct map *tags;
    struct map *fields;
    struct metric_time time;
};

struct metric *metric_new();
void metric_new_full(const char *name, struct map *tags, struct map *fields, struct metric_time time);
struct list *metric_parse(const char *buf);
char *metric_serialize(struct list *metrics);
bool metric_equal(struct metric *metric1, struct metric *metric2, int mode);
char *metric_get_name(struct metric *metric);
void metric_set_name(struct metric *metric, char *name);
struct list * metric_tags(struct metric *metric);
void metric_add_tag(struct metric *metric, const char *tag, const char *value);
char *metric_get_tag(struct metric *metric, const char *tag);
bool metric_update_tag(struct metric *metric, const char *tag, const char *value);
bool metric_remove_tag(struct metric *metric, const char *tag);
struct list *metric_fields(struct metric *metric);
void metric_add_field(struct metric *metric, const char *field, struct metric_value *value);
struct metric_value *metric_get_field(struct metric *metric, const char *field);
bool metric_update_field(struct metric *metric, const char *field, struct metric_value *value);
bool metric_remove_field(struct metric *metric, const char *field);
struct metric_time metric_get_time(struct metric *metric);
void metric_set_time(struct metric *metric, struct metric_time time);

#endif
