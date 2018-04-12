#include <string.h>

#include "json.h"
#include "map.h"

struct json *json_object() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_OBJECT_TYPE;
    j->data = map_new(strcmp);
    return j;
}

struct json *json_array() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_ARRAY_TYPE;
    j->data = malloc(sizeof(struct json_array));
    j->data->size = 0;
    j->data->data = NULL;
    return j;
}

struct json *json_string(const char *value) {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_STRING_TYPE;
    j->data = strdup(value);
    return j;
}

struct json *json_integer(long long value) {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_INTEGER_TYPE;
    j->data = malloc(sizeof(long long));
    *(long long *)j->data = value;
    return j;
}

struct json *json_real(double value) {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_REAL_TYPE;
    j->data = malloc(sizeof(double));
    *(double *)j->data = value;
    return j;
}
struct json *json_true() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_TRUE_TYPE;
    j->data = malloc(sizeof(bool));
    *(bool *)j->data = true;
    return j;
}

struct json *json_false() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_FALSE_TYPE;
    j->data = malloc(sizeof(bool));
    *(bool *)j->data = false;
    return j;
}

struct json *json_null() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_NULL_TYPE;
    j->data = NULL;
    return j;
}

struct json *json_loads(const char *s);

const char *json_dumps(struct json *j) {
    if (j->type == JSON_OBJECT_TYPE) {
        
    }
}

bool json_equal(struct json *j1, struct json *j2);

void json_destroy(struct json *j) {
    if (j->type == JSON_OBJECT_TYPE) {
        struct list *p, *keys = json_object_keys(j);
        for (p = keys; p != NULL; p = list_next(p)) {
            void *orig_key, *orig_data;
            if(map_remove(j->data, p, &orig_key, &orig_data)) {
                free(orig_key);
                json_destroy(orig_data);
            }
        }
        map_destroy(j->data);
        list_destroy(keys);
        free(j);
    } else if (j->type == JSON_ARRAY_TYPE) {
        struct json *a = j->data;
        for (int i = 0; i < a->size; i++) {
            json_destroy(a->data[i]);
        }
        free(a->data);
        free(j);
    } else if (j->type == JSON_NULL_TYPE) {
        free(j);
    } else {
        /* string, integer, real, true, false */
        free(j->data);
        free(j);
    }
}

/**
 * get json object keys.

 * @return a list of keys. the must be destroyed by list_destroy().
 */
struct list *json_object_keys(struct json *object) {
    return map_keys(object->data);
}

/**
 * add a key-value pair to a json object.
 *
 * if the key already exists, then the value will be updated.
 */
void json_object_add(struct json *object, const char *key, struct json *value) {
    void *orig_key, *orig_data;
    if (map_has(object->data, key)) {
        map_remove(object->data, key, &orig_key, &orig_data);
        free(orig_key);
        json_destroy(orig_data);
    }
    map_add(object->data, key, value);
}

struct json *json_object_get(struct json *object, const char *key) {
    void *data;
    if (map_get(object->data, key, &data))
        return data;
    return NULL;
}

void json_object_remove(struct json *object, const char *key) {
    void *orig_key, *orig_data;
    if (map_remove(object->data, key, &orig_key, &orig_data)) {
        free(orig_key);
        json_destroy(orig_data);
    }
}

size_t json_array_size(struct json *array) {
    struct json_array *a = array->data;
    return a->size;
}

void json_array_append(struct json *array, struct json *value) {
    struct json_array *a = array->data;
    a->data = realloc(sizeof(struct json) * (a->size + 1));
    a->data[a->size] = value;
    a->size += 1;
}

struct json *json_array_get(struct json *array, size_t index) {
    struct json_array *a = array->data;
    if (index >= a->size)
        return NULL;
    return a->data[index];
}

void json_array_remove(struct json *array, size_t index) {
    struct json_array *a = array->data;
    if (index >= a->size)
        return
    json_destroy(a->data[index]);
    for (size_t i = index; i < a->size - 1; i++) {
        a->data[i] = a->data[i + 1];
    }
    a->size -= 1;
    a->data = realloc(a->data, sizeof(struct json) * a->size);
}
