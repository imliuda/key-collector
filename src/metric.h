#ifndef __OSCLT_METRIC_H__
#define __OSCLT_METRIC_H__

#include <time.h>

#include "json.h"

#define METRIC_EQUAL_NAME     1
#define METRIC_EQUAL_TAGS     2
#define METRIC_EQUAL_VALUE    4
#define METRIC_EQUAL_TIME     8

struct metric {
    const char *name;
    struct json *tags;
    struct json *value;
    struct timespec time;
};

struct metric *metric_new();
void metric_destroy(struct metric *m);
struct list *metric_parse(const char *buf);
const char *metric_serialize(struct metric *m);
bool metric_equal(struct metric *m1, struct metric *m2, int mode);
void metric_set_name(struct metric *m, const char *name);
const char *metric_get_name(struct metric *m);
struct list *metric_tag_keys(struct metric *m);
void metric_add_tag(struct metric *m, const char *key, const char *value);
const char *metric_get_tag(struct metric *m, const char *key);
void metric_set_value(struct metric *m, struct json *value);
struct json *metric_get_value(struct metric *m);
void metric_set_time(struct metric *m, struct timespec time);
struct timespec metric_get_time(struct metric *m);

#endif
