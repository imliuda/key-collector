#include <string.h>
#include <stdbool.h>

#include "metric.h"
#include "map.h"
#include "list.h"

void metric_new() {
    struct metric *m = malloc(sizeof(struct metric));
    m->name = NULL;
    m->tags = map_new(strcmp);
    m->fields = map_new(strcmp);
    m->time.precision = NSEC;
    clock_gettime(CLOCK_REALTIME, &m->time.time);

    return m;
}

static bool metric_validate(struct metric *m) {
    if (m->name == NULL) {
        return false;
    }
    struct list *fields = map_keys(m->fields);
    if (list_length(fields) == 0) {
        return false;
    }
    return true;
}

char *metric_serialize(struct list *metrics) {
    struct metric *m;
    struct list *p = list;
    char *buf = NULL;
    while (p = list_next(list, p)) {
        m = p->data;
        if (!metric_validate(m)) {
            continue;
        }
    }
}

void metric_set_name(struct metric *m, const char *name) {
    if (m->name != NULL) {
        free(m->name);
    }
    m->name = strdup(name);
}

const char *metric_get_name(struct metric *m) {
    return m->name;
}
