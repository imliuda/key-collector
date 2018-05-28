#include <string.h>
#include <stddef.h>
#include <wchar.h>

#include "json.h"
#include "map.h"
#include "str.h"

const char *json_error_text[] = {
    [JSON_ERROR_SUCCESS] = "success",
    [JSON_ERROR_UNKNOWN_VALUE] = "unknown value type",
    [JSON_ERROR_INVALID_ENCODING] = "invalid encoding",
    [JSON_ERROR_EMPTY_JSON] = "empty json content",
    [JSON_ERROR_INVALID_NAME] = "invalid name",
    [JSON_ERROR_EXPECTING_NAME] = "expecting a name",
    [JSON_ERROR_EXPECTING_VALUE] = "expecting a value",
    [JSON_ERROR_EXPECTING_COMMA] = "expecting comma seperator",
    [JSON_ERROR_UNNECESSARY_COMMA] = "unnecessary comma seperator",
    [JSON_ERROR_EXPECTING_COLON] = "expecting colon seperator"
};

static int keycmp(void *k1, void *k2) {
    return strcmp((const char *)k1, (const char *)k2);
}

/**
 * create json object.
 */
struct json *json_object() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_OBJECT_TYPE;
    j->data = map_new(keycmp);
    j->nref = 1;
    return j;
}

/**
 * create json array.
 */
struct json *json_array() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_ARRAY_TYPE;
    struct json_array *a = malloc(sizeof(struct json_array));
    a->size = 0;
    a->data = NULL;
    j->data = a;
    j->nref = 1;
    return j;
}

/**
 * create json string.
 */
struct json *json_string(const char *value) {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_STRING_TYPE;
    j->data = strdup(value);
    j->nref = 1;
    return j;
}

/**
 * create json integer.
 */
struct json *json_integer(long long value) {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_INTEGER_TYPE;
    j->data = malloc(sizeof(long long));
    *(long long *)j->data = value;
    j->nref = 1;
    return j;
}

/**
 * create json real.
 */
struct json *json_real(double value) {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_REAL_TYPE;
    j->data = malloc(sizeof(double));
    *(double *)j->data = value;
    j->nref = 1;
    return j;
}

/**
 * create json true.
 */
struct json *json_true() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_TRUE_TYPE;
    j->data = malloc(sizeof(bool));
    *(bool *)j->data = true;
    j->nref = 1;
    return j;
}

/**
 * create json false.
 */
struct json *json_false() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_FALSE_TYPE;
    j->data = malloc(sizeof(bool));
    *(bool *)j->data = false;
    j->nref = 1;
    return j;
}

/**
 * create json null.
 */
struct json *json_null() {
    struct json *j = malloc(sizeof(struct json));
    j->type = JSON_NULL_TYPE;
    j->data = NULL;
    j->nref = 1;
    return j;
}

/**
 * create json value by referencing another value.
 *
 * json_object(), json_array(), json_string(), json_integer()
 * json_real(), json_true(), json_false(), json_null() functions
 * create a json value, which must be freed by json_destroy().
 * these values can be used only once(add to an object or array by once).
 * if you need to use it multi times, e.g, add a value "jv" to an object
 * and an array like this:
 *     json_object_add(obj, jv);
 *     json_array_append(arr, jv);
 * you need to do as follows:
 *     json_object_add(obj, jv);
 *     json_object_add(obj, json_ref(jv));
 * json_ref() increase a json value's reference count. when json_destroy()
 * is called, first will decrease reference count, and if count is 0, then
 * destroy it, else do nothing.
 */
struct json *json_ref(struct json *j) {
    j->nref += 1;
    return j;
}

/**
 * space, horizontal tab, new line, carriage return
 */
static inline bool json_is_ws(wchar_t c) {
    return c == 0x20 || c == 0x09 || c == 0x0a || c == 0x0d;
}

static inline void json_skip_ws(struct json_parser *p) {
    wchar_t *buffer = p->buffer;
    size_t offset = p->offset;
    size_t length = p->length;

    while (json_is_ws(buffer[offset]) && offset < length)
        offset++;
    p->offset = offset;
}

static inline bool json_is_ctrl(wchar_t c) {
    return c >= 0x0 && c <= 0x1f;
}

static inline bool json_parse_done(struct json_parser *p) {
    return p->offset == p->length;
}

static void json_parse_error(struct json_parser *p, struct json_error *error, enum json_error_code code) {
    if (error == NULL) {
        return;
    }
    error->code = code;
    error->text = json_error_text[code];
    error->position = p->offset + 1;
    error->line = 1;
    error->column = 1;
    wchar_t *c = p->buffer + p->offset;
    while (c > p->buffer) {
        c--;
        error->column++; 
        if (*c == 0xa) { /* new line '\n' */
            break;
        }
    }
    while (c > p->buffer) {
        c--;
        if (*c == 0xa) {
            error->line++;
        }
    }
}

static struct json *json_parse_object(struct json_parser *p, struct json_error *error);
static struct json *json_parse_array(struct json_parser *p, struct json_error *error);
static struct json *json_parse_string(struct json_parser *p);
static struct json *json_parse_number(struct json_parser *p);
static struct json *json_parse_true(struct json_parser *p);
static struct json *json_parse_false(struct json_parser *p);
static struct json *json_parse_null(struct json_parser *p);

struct json *json_parse_null(struct json_parser *p) {
    if (p->offset <= p->length - 4 && p->buffer[p->offset] == 0x6e &&
        p->buffer[p->offset + 1] == 0x75 && p->buffer[p->offset + 2] == 0x6c &&
        p->buffer[p->offset + 3] == 0x6c) {
        p->offset += 4;
        return json_null();
    }
    return NULL;
}

struct json *json_parse_false(struct json_parser *p) {
    if (p->offset <= p->length - 5 && p->buffer[p->offset] == 0x66 &&
        p->buffer[p->offset + 1] == 0x61 && p->buffer[p->offset + 2] == 0x6c &&
        p->buffer[p->offset + 3] == 0x73 && p->buffer[p->offset + 4] == 0x65) {
        p->offset += 5;
        return json_false();
    }
    return NULL;
}

struct json *json_parse_true(struct json_parser *p) {
    if (p->offset <= p->length - 4 && p->buffer[p->offset] == 0x74 &&
        p->buffer[p->offset + 1] == 0x72 && p->buffer[p->offset + 2] == 0x75 &&
        p->buffer[p->offset + 3] == 0x65) {
        p->offset += 4;
        return json_true();
    }
    return NULL;
}

struct json *json_parse_number(struct json_parser *p) {
    size_t offset = p->offset;
    size_t start = p->offset, end = p->offset;

    wchar_t *wcs;
    char *s;
    size_t frac, exp;

    if (p->buffer[p->offset] == 0x2d) { /* minus '-' */
        p->offset++;
    }

    if (json_parse_done(p)) {
        p->offset = offset;
        return NULL;
    }

    if (p->buffer[p->offset] == 0x30) { /* leading zero */
        p->offset++;
        if (json_parse_done(p)) {
            return json_integer(0);
        } else { /* frac or exp */
            if (p->buffer[p->offset] == 0x2e) { /* decimal-point '.' */
parse_real:
                p->offset++;
                frac = p->offset;
                while (!json_parse_done(p)) {
                    if (p->buffer[p->offset] >= 0x30 && p->buffer[p->offset] <= 0x39) {
                        p->offset++;
                        end = p->offset;
                    } else {
                        break;
                    }
                }
                if (p->offset == frac) { /* frac DIGIT not found, we got "x." */
                    p->offset = offset;
                    return NULL;
                }

                json_skip_ws(p);
                if (json_parse_done(p) || p->buffer[p->offset] == 0x5d ||
                    p->buffer[p->offset] == 0x7d || p->buffer[p->offset] == 0x2c) { /* got "x.xx" */
                    goto return_real;
                } else if (p->buffer[p->offset] == 0x65 || p->buffer[p->offset] == 0x45) { /* got "x.xxe" or "x.xxE" */
                    p->offset++;
                    if (json_parse_done(p)) {
                        p->offset = offset;
                        return NULL;
                    }
                    if (p->buffer[p->offset] == 0x2b || p->buffer[p->offset] == 0x2d) { /* got "x.xxE-" or "x.xxE+" */
                        p->offset++;
                    }
                    exp = p->offset;
                    while (!json_parse_done(p)) {
                        if (p->buffer[p->offset] >= 0x30 && p->buffer[p->offset] <= 0x39) {
                            p->offset++;
                            end = p->offset;
                        } else {
                            break;
                        }
                    }
                    if (p->offset == exp) { /* exp DIGIT not found, got "x.xx(e|E)[+|-]" */
                        p->offset = offset;
                        return NULL;
                    }

                    json_skip_ws(p);
                    if (json_parse_done(p) || p->buffer[p->offset] == 0x5d ||
                        p->buffer[p->offset] == 0x7d || p->buffer[p->offset] == 0x2c) { /* got "x.xx(e|E)[+|-]xx" */
                        goto return_real;
                    } else {
                        p->offset = offset;
                        return NULL;
                    }
                }
            } else {
                json_skip_ws(p);
                if (p->buffer[p->offset] == 0x5d || p->buffer[p->offset] == 0x7d ||
                    p->buffer[p->offset] == 0x2c) { /* end at ']', '}', ',' */
                    return json_integer(0);
                } else {
                    p->offset = offset;
                    return NULL;
                }
            }
        }
    } else { /* decimal */
        while (!json_parse_done(p)) {
            if (p->buffer[p->offset] >= 0x30 && p->buffer[p->offset] <= 0x39) {
                p->offset++;
                end = p->offset;
            } else if (p->buffer[p->offset] == 0x2e || p->buffer[p->offset] == 0x65 ||
                       p->buffer[p->offset] == 0x45) { /* '.' or 'e' or 'E' */
                goto parse_real;
            } else {
                break;
            }
        }
        json_skip_ws(p);
        if (json_parse_done(p) || p->buffer[p->offset] == 0x5d ||
            p->buffer[p->offset] == 0x7d || p->buffer[p->offset] == 0x2c) {
            wchar_t *wcs = wcsndup(p->buffer + start, end - start);
            char *s = strutf8enc(wcs);
            long long integer = strtoll(s, NULL, 10);
            free(wcs);
            free(s);
            return json_integer(integer);
        } else {
            p->offset = offset;
            return NULL;
        }
    }

return_real:
    wcs = wcsndup(p->buffer + start, end - start);
    s = strutf8enc(wcs);
    double real = atof(s);
    free(wcs);
    free(s);
    return json_real(real);
}

struct json *json_parse_string(struct json_parser *p) {
    size_t offset = p->offset;

    if (p->buffer[p->offset] != 0x22) { /* qoutation mark */
        return NULL;
    }

    /* skip '"' */
    p->offset++;
    size_t start = p->offset, end = p->offset;
    while (!json_parse_done(p)) { /* end quotation mark */
        if (p->buffer[p->offset] == 0x22 && p->buffer[p->offset - 1] != 0x5c) {
            p->offset++;
            break;
        }
        end++;
        p->offset++;
    }

    if (start == end) { /* json parse done */
        p->offset = offset;
        return NULL;
    }

    wchar_t *wcs = wcsndup(p->buffer + start, end - start);
    char *s = strutf8enc(wcs);
    struct json *string = json_string(s);
    free(wcs);
    free(s);
    return string;
}

struct json *json_parse_simple(struct json_parser *p) {
    struct json *value;
    if (value = json_parse_string(p)) {
        return value;
    } else if (value = json_parse_number(p)) {
        return value;
    } else if (value = json_parse_true(p)) {
        return value;
    } else if (value = json_parse_false(p)) {
        return value;
    } else if (value = json_parse_null(p)) {
        return value;
    }
    return NULL;
}

struct json *json_parse_array(struct json_parser *p, struct json_error *error) {
    struct json *value = NULL;
    wchar_t c;
    /* skip '[' */
    p->offset++;

    json_skip_ws(p);
    if (!json_parse_done(p) && p->buffer[p->offset] == 0x5d) {
        p->offset++;
        return json_array();
    } else if (json_parse_done(p)) {
        json_parse_error(p, error, JSON_ERROR_EXPECTING_VALUE);
        return NULL;
    }

    struct json *array = json_array();

parse_element:
    value = NULL;
    c = p->buffer[p->offset];
    if (c == 0x7b) { /* object value '{' */
        value = json_parse_object(p, error);
    } else if (c == 0x5b) { /* array value '[' */
        value = json_parse_array(p, error);
    } else {
        value = json_parse_simple(p);
    }

    if (!value) {
        json_destroy(array);
        json_parse_error(p, error, JSON_ERROR_UNKNOWN_VALUE);
        /* error has been set in parsing object or array */
        return NULL;
    }

    json_array_append(array, value);

     /* finish an array element, find a ',' or ']' */
    json_skip_ws(p);
    if (p->buffer[p->offset] == 0x2c) { /* comma */
        p->offset++;
        json_skip_ws(p);
        if (p->buffer[p->offset] == 0x5d) { /* found ']', additional comma */
            json_destroy(array);
            json_parse_error(p, error, JSON_ERROR_UNNECESSARY_COMMA);
            return NULL;
        } else {
            /* parse next name value pair */
            goto parse_element;
        }
    } else if (p->buffer[p->offset] == 0x5d) { /* end array */
        p->offset++;
        return array;
    } else {
        json_destroy(array);
        json_parse_error(p, error, JSON_ERROR_EXPECTING_COMMA);
        return NULL;
    }
}

struct json *json_parse_object(struct json_parser *p, struct json_error *error) {
    struct json *name;
    wchar_t c;

    /* skip '{' */
    p->offset++;

    json_skip_ws(p);
    if (!json_parse_done(p) && p->buffer[p->offset] == 0x7d) { /* empty object '}' */
        p->offset++;
        return json_object();
    } else if (json_parse_done(p)) {
        json_parse_error(p, error, JSON_ERROR_EXPECTING_NAME);
        return NULL;
    }

    /* create a new object */
    struct json *object = json_object();

parse_name_value:
    /* parse json key */
    name = json_parse_string(p);

    if (!name) {
        json_destroy(object);
        json_parse_error(p, error, JSON_ERROR_INVALID_NAME);
        return NULL;
    }


    json_skip_ws(p);
    if (json_parse_done(p) || p->buffer[p->offset] != 0x3a) { /* colon ':' */
        json_destroy(object);
        json_destroy(name);
        json_parse_error(p, error, JSON_ERROR_EXPECTING_COLON);
        return NULL;
    }

    // skip colon
    p->offset++;
    json_skip_ws(p);
    if (json_parse_done(p)) {
        json_destroy(object);
        json_destroy(name);
        json_parse_error(p, error, JSON_ERROR_EXPECTING_VALUE);
        return NULL;
    }

    c = p->buffer[p->offset];
    if (c == 0x7d) { /* end of object '}' */
        p->offset++;
        return object;
    } else if (c == 0x7b) { /* object value '{' */
        struct json *value = json_parse_object(p, error);
        if (value) {
            /* add child object to current */
            json_object_add(object, json_string_value(name), value);
            json_destroy(name);
        } else {
            json_destroy(object);
            json_destroy(name);
            /* error has been set when passing object value */
            return NULL;
        }
    } else if (c == 0x5b) { /* array value '[' */
        struct json *value = json_parse_array(p, error);
        if (value) {
            json_object_add(object, json_string_value(name), value);
            json_destroy(name);
        } else {
            json_destroy(object);
            json_destroy(name);
            /* error has been set when passing array value */
            return NULL;
        }
    } else { /* simple value */
        struct json *simple = json_parse_simple(p);
        if (simple) {
            json_object_add(object, json_string_value(name), simple);
            json_destroy(name);
        } else { /* unknown value type */
            json_destroy(object);
            json_destroy(name);
            return NULL;
        }
    }

    /* finish a name-value pair, find a ',' or '}' */
    json_skip_ws(p);
    if (p->buffer[p->offset] == 0x2c) { /* comma */
        p->offset++;
        json_skip_ws(p);
        if (p->buffer[p->offset] == 0x7d) { /* found '}', additional comma */
            json_destroy(object);
            json_destroy(name);
            json_parse_error(p, error, JSON_ERROR_UNNECESSARY_COMMA);
            return NULL;
        } else {
            /* parse next name value pair */
            goto parse_name_value;
        }
    } else if (p->buffer[p->offset] == 0x7d) { /* end object '}' */
        p->offset++;
        return object;
    } else {
        json_destroy(object);
        json_destroy(name);
        json_parse_error(p, error, JSON_ERROR_EXPECTING_COMMA);
        return NULL;
    }
}

static struct json *json_parse(struct json_parser *p, struct json_error *error) {
    json_skip_ws(p);
    if (json_parse_done(p)) {
        json_parse_error(p, error, JSON_ERROR_EMPTY_JSON);
        return NULL;
    }

    wchar_t c = p->buffer[p->offset];
    if (c == 0x7b) { /* object start '{' */
        return json_parse_object(p, error);
    } else if (c == 0x5b) { /* array start '[' */
        return json_parse_array(p, error);
    } else { /* json simple value: string, integer, real, true, false, null */
        return json_parse_simple(p);
    }
}

/**
 * parse a json string to a json value.
 */
struct json *json_loads(const char *s, struct json_error *error) {
    if (error) {
        error->code = JSON_ERROR_SUCCESS;
        error->text = json_error_text[JSON_ERROR_SUCCESS];
        error->line = error->column = error->position = 0;
    }
    wchar_t *wcs = strutf8dec(s);
    if (!wcs) {
        if (error) {
            error->code = JSON_ERROR_INVALID_ENCODING;
            error->text = json_error_text[JSON_ERROR_INVALID_ENCODING];
            error->line = error->column = error->position = 0;
        }
        return NULL;
    }
    struct json_parser pb;
    pb.buffer = wcs;
    pb.offset = 0;
    pb.length = wcslen(wcs);
    struct json *j = json_parse(&pb, error);
    free(wcs);
    return j;
}

void json_dumps_internal(struct json *j, struct strbuf *sb, int level, bool last) {
    if (j->type == JSON_OBJECT_TYPE) {
        strbufexts(sb, "{\n");

        struct list *p, *keys = json_object_keys(j);
        for (p = keys; p != NULL; p = list_next(p)) {
            for (int i = 0; i <= level; i++)
                strbufexts(sb, "\t");
            strbufexts(sb, "\"");
            strbufexts(sb, list_data(p));
            strbufexts(sb, "\"");
            strbufexts(sb, ": ");
            struct json *value = json_object_get(j, list_data(p));
            json_dumps_internal(value, sb, level + 1, p->next == NULL);
        }
        list_destroy(keys);

        for (int i = 0; i < level; i++)
            strbufexts(sb, "\t");
        strbufexts(sb, "}");
        if (!last)
            strbufexts(sb, ",");
        strbufexts(sb, "\n");
    } else if (j->type == JSON_ARRAY_TYPE) {
        strbufexts(sb, "[\n");

        size_t array_size = json_array_size(j);
        for (int i = 0; i < array_size; i++) {
            for (int i = 0; i <= level; i++)
                strbufexts(sb, "\t");

            json_dumps_internal(json_array_get(j, i), sb, level + 1, i == array_size - 1);
        }

        for (int i = 0; i < level; i++)
            strbufexts(sb, "\t");
        strbufexts(sb, "]");
        if (!last)
            strbufexts(sb, ",");
        strbufexts(sb, "\n");
    } else if (j->type == JSON_STRING_TYPE) {
        strbufexts(sb, "\"");
        strbufexts(sb, j->data);
        strbufexts(sb, "\"");
        if (!last)
            strbufexts(sb, ",");
        strbufexts(sb, "\n");
    } else if (j->type == JSON_INTEGER_TYPE) {
        size_t len = snprintf(NULL, 0, "%lld", *(long long *)j->data);
        char buf[len + 1];
        snprintf(buf, len + 1, "%lld", *(long long *)j->data);
        strbufexts(sb, buf);
        if (!last)
            strbufexts(sb, ",");
        strbufexts(sb, "\n");
    } else if (j->type == JSON_REAL_TYPE) {
        size_t len = snprintf(NULL, 0, "%f", *(double *)j->data);
        char buf[len + 1];
        snprintf(buf, len + 1, "%f", *(double *)j->data);
        strbufexts(sb, buf);
        if (!last)
            strbufexts(sb, ",");
        strbufexts(sb, "\n");
    } else if (j->type == JSON_TRUE_TYPE) {
        strbufexts(sb, "true");
        if (!last)
            strbufexts(sb, ",");
        strbufexts(sb, "\n");
    } else if (j->type == JSON_FALSE_TYPE) {
        strbufexts(sb, "false");
        if (!last)
            strbufexts(sb, ",");
        strbufexts(sb, "\n");
    } else if (j->type == JSON_NULL_TYPE) {
        strbufexts(sb, "null");
        if (!last)
            strbufexts(sb, ",");
        strbufexts(sb, "\n");
    }
}

/**
 * dump a json value to a json string.
 */
char *json_dumps(struct json *j) {
    struct strbuf *sb = strbufnew(512);
    json_dumps_internal(j, sb, 0, true);
    const char *s = strbufstr(sb);
    /* remove last '\n' */
    char *r = strndup(s, strlen(s) - 1);
    strbuffree(sb);
    return r;
}

/**
 * compare if two json values are equal.
 */
bool json_equal(struct json *j1, struct json *j2);

/**
 * destroy a json value.
 */
void json_destroy(struct json *j) {
    if (--j->nref > 0)
        return;
    if (j->type == JSON_OBJECT_TYPE) {
        struct list *p, *keys = json_object_keys(j);
        for (p = keys; p != NULL; p = list_next(p)) {
            void *orig_key, *orig_data;
            map_remove(j->data, list_data(p), &orig_key, &orig_data);
            free(orig_key);
            json_destroy(orig_data);
        }
        list_destroy(keys);
        map_destroy(j->data);
        free(j);
    } else if (j->type == JSON_ARRAY_TYPE) {
        struct json_array *a = j->data;
        for (int i = 0; i < a->size; i++) {
            json_destroy(a->data[i]);
        }
        free(a->data);
        free(a);
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
    if (map_has(object->data, (void *)key)) {
        map_remove(object->data, (void *)key, &orig_key, &orig_data);
        free(orig_key);
        json_destroy(orig_data);
    }
    map_add(object->data, (void *)strdup(key), value);
}

/**
 * get object value.
 */
struct json *json_object_get(struct json *object, const char *key) {
    void *data;
    if (map_get(object->data, (void *)key, &data))
        return data;
    return NULL;
}

/**
 * remove a value from an object.
 */
void json_object_remove(struct json *object, const char *key) {
    void *orig_key, *orig_data;
    if (map_remove(object->data, (void *)key, &orig_key, &orig_data)) {
        free(orig_key);
        json_destroy(orig_data);
    }
}

/**
 * get json array size.
 */
size_t json_array_size(struct json *array) {
    struct json_array *a = array->data;
    return a->size;
}

/**
 * append a new value to json array.
 */
void json_array_append(struct json *array, struct json *value) {
    struct json_array *a = array->data;
    a->data = realloc(a->data, sizeof(struct json *) * (a->size + 1));
    a->data[a->size] = value;
    a->size += 1;
}

/**
 * get array index value.
 */
struct json *json_array_get(struct json *array, size_t index) {
    struct json_array *a = array->data;
    if (index >= a->size)
        return NULL;
    return a->data[index];
}

/**
 * remove a index value from json array.
 */
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
