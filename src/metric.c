#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "metric.h"
#include "json.h"
#include "list.h"

struct metric *metric_new() {
    struct metric *m = malloc(sizeof(struct metric));
    m->data = json_object();
    return m;
}

static bool metric_validate(struct metric *m) {
    if (!m || !m->data) {
        return false;
    }

    struct json *name = json_object_get(m->data, "metric");
    if (!name || !json_is_string(name)) {
        return false;
    }

    struct json *tags = json_object_get(m->data, "tag");
    if (tags && json_is_object(tags)) {
        struct list *t, *ts = json_object_keys(tags);
        for (t = ts; t != NULL; t = list_next(t)) {
            struct json *tv = json_object_get(tags, list_data(t));
            if (!json_is_string(tv)) {
                return false;
            }
        }
        list_destroy(ts);
    }

    if (!json_object_get(m->data, "value")) {
        return false;
    }

    return true;
}

struct list *metric_loads(const char *buf) {
    struct json *j;
    if (j = json_loads(buf, NULL)) {
        struct list *list = list_new();
        if (json_is_array(j)) {
            for (size_t i = 0; i < json_array_size(j); i++) {
                struct json *m = json_array_get(j, i);
                struct metric tm;
                tm.data = m;
                if (metric_validate(&tm)) {
                    if (!json_object_get(m, "time")) {
                        time_t timestamp = time(NULL);
                        json_object_add(m, "time", json_integer(timestamp));
                    }
                    struct metric *metric = malloc(sizeof(struct metric));
                    metric->data = json_ref(m);
                    list = list_append(list, metric);
                }
            }
        } else if (json_is_object(j)) {
            struct metric m;
            m.data = j;
            if (metric_validate(&m)) {
                struct metric *metric = malloc(sizeof(struct metric));
                metric->data = json_ref(j);
                list = list_append(list, metric);
            }
        }

        json_destroy(j);

        if (list_length(list) > 0)
            return list;
        list_destroy(list);
        return NULL;
    }
    return NULL;
}

char *metric_dumps(struct metric *m) {
    if (!m || !metric_validate(m))
        return NULL;
    if (!json_object_get(m->data, "time")) {
        time_t timestamp = time(NULL);
        json_object_add(m->data, "time", json_integer(timestamp));
    }
    return json_dumps(m->data);
}

char *metric_list_dumps(struct list *ms) {
    if (!ms)
        return NULL;
    struct list *p;
    struct metric *m;
    struct json *array = json_array();
    time_t timestamp = time(NULL);
    for (p = ms; p != NULL; p = list_next(p)) {
        m = list_data(p);
        if (metric_validate(m)) {
            if (!json_object_get(m->data, "time")) {
                json_object_add(m->data, "time", json_integer(timestamp));
            }
            json_array_append(array, json_ref(m->data));
        }
    }
    if (json_array_size(array) > 0) {
        char *r = json_dumps(array);
        json_destroy(array);
        return r;
    }
    json_destroy(array);
    return NULL;
}

void metric_set_name(struct metric *m, const char *name) {
    struct json *jname;
    if (jname = json_object_get(m->data, "metric")) {
        json_destroy(jname);
    }
    json_object_add(m->data, "metric", json_string(name));
}

const char *metric_get_name(struct metric *m) {
    struct json *jname;
    if (jname = json_object_get(m->data, "metric")) {
        return json_string_value(jname);
    }
    return NULL;
}

struct list *metric_tag_keys(struct metric *m) {
    if (!m)
        return NULL;
    struct json *tags = json_object_get(m->data, "tag");
    if (tags && json_is_object(tags))
        return json_object_keys(tags);
    return NULL;
}

void metric_add_tag(struct metric *m, const char *key, const char *value) {
    if (!m)
        return;
    struct json *tags = json_object_get(m->data, "tag");
    if (!tags) {
        tags = json_object();
        json_object_add(m->data, "tag", tags);
    }
    if (json_is_object(tags)) {
        json_object_add(tags, key, json_string(value));
    }
}

const char *metric_get_tag(struct metric *m, const char *key) {
    if (!m)
        return NULL;
    struct json *tags = json_object_get(m->data, "tag");
    if (!tags)
        return NULL;
    struct json *tv = json_object_get(tags, key);
    if (!tv)
        return NULL;
    return json_string_value(tv);
}

void metric_set_value(struct metric *m, struct json *value) {
    if (!m)
        return;
    if (json_object_get(m->data, "value")) {
        json_object_remove(m->data, "value");
    }
    json_object_add(m->data, "value", value);
}

struct json *metric_get_value(struct metric *m) {
    if (!m)
        return NULL;
    struct json *value = json_object_get(m->data, "value");
    return value ? value : NULL;
}

void metric_set_time(struct metric *m, time_t time) {
    if (!m)
        return;
    if (json_object_get(m->data, "time")) {
        json_object_remove(m->data, "time");
    }
    json_object_add(m->data, "time", json_integer(time));
}

time_t metric_get_time(struct metric *m) {
    if (!m)
        return 0;
    struct json *time = json_object_get(m->data, "time");
    return time ? json_integer_value(time) : 0;
}

void metric_destroy(struct metric *m) {
    if (m) {
        json_destroy(m->data);
        free(m);
    }
}

void metric_list_destroy(struct list *ms) {
    if (ms) {
        struct list *p;
        for (p = ms; p != NULL; p = list_next(p)) {
            metric_destroy(list_data(p));
        }
        list_destroy(ms);
    }
}
