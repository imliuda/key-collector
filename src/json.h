#ifndef __OSCLT_JSON_H__
#define __OSCLT_JSON_H__

enum json_type {
    JSON_OBJECT_TYPE,
    JSON_ARRAY_TYPE,
    JSON_STRING_TYPE,
    JSON_INTEGER_TYPE,
    JSON_REAL_TYPE,
    JSON_TRUE_TYPE,
    JSON_FALSE_TYPE,
    JSON_NULL_TYPE
};

struct json {
    enum json_type type;
    void *data;
}

struct json_array {
    size_t size;
    struct json *data;
};

#define json_typeof(json)      ((json)->type)
#define json_is_object(json)   ((json) && json_typeof(json) == JSON_OBJECT_TYPE)
#define json_is_array(json)    ((json) && json_typeof(json) == JSON_ARRAY_TYPE)
#define json_is_string(json)   ((json) && json_typeof(json) == JSON_STRING_TYPE)
#define json_is_integer(json)  ((json) && json_typeof(json) == JSON_INTEGER_TYPE)
#define json_is_real(json)     ((json) && json_typeof(json) == JSON_REAL_TYPE)
#define json_is_true(json)     ((json) && json_typeof(json) == JSON_TRUE_TYPE)
#define json_is_false(json)    ((json) && json_typeof(json) == JSON_FALSE_TYPE)
#define json_is_null(json)     ((json) && json_typeof(json) == JSON_NULL_TYPE)

struct json *json_object();
struct json *json_array();
struct json *json_string(const char *value);
struct json *json_integer(long long value);
struct json *json_real(double value);
struct json *json_true();
struct json *json_false();
struct json *json_null();

struct json *json_loads(const char *s);
const char *json_dumps(struct json *j);
bool json_equal(struct json *j1, struct json *j2);
void json_destroy(struct json *j);

struct list *json_object_keys(struct json *object);
void json_object_add(struct json *object, const char *key, struct json *value);
struct json *json_object_get(struct json *object, const char *key);
void json_object_remove(struct json *object, const char *key);

size_t json_array_size(struct json *array);
void json_array_append(struct json *array, struct json *value);
struct json *json_array_get(struct json *array, size_t index);
void json_array_remove(struct json *array, size_t index);

#endif
