#ifndef __OSCLT_METRIC_H__
#define __OSCLT_METRIC_H__

#include <time.h>
#include <wchar.h>

#include "json.h"

#define METRIC_EQUAL_NAME     1
#define METRIC_EQUAL_TAGS     2
#define METRIC_EQUAL_VALUE    4
#define METRIC_EQUAL_TIME     8

struct metric {
    struct json *data;
};

struct metric *metric_new();
void metric_destroy(struct metric *m);
void metric_list_destroy(struct list *ms);
struct list *metric_loads(const char *buf);
char *metric_dumps(struct metric *m);
char *metric_list_dumps(struct list *ms);
bool metric_equal(struct metric *m1, struct metric *m2, int mode);
void metric_set_name(struct metric *m, const char *name);
const char *metric_get_name(struct metric *m);
struct list *metric_tag_keys(struct metric *m);
void metric_add_tag(struct metric *m, const char *key, const char *value);
const char *metric_get_tag(struct metric *m, const char *key);
void metric_set_value(struct metric *m, struct json *value);
struct json *metric_get_value(struct metric *m);
void metric_set_time(struct metric *m, time_t time);
time_t metric_get_time(struct metric *m);

#endif
