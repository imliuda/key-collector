#ifndef __OSCLT_METRIC_H__
#define __OSCLT_METRIC_H__

#include <time.h>

#include "map.h"

struct metric {
    char *name;
    struct map *tags;
    struct map *fields;
    struct timespec time;
};

// clock_gettime(clockid_t clk_id, struct timespec *tp);
// CLOCK_REALTIME

void metric_parse(const char *buf, struct list **metrics);

#endif
