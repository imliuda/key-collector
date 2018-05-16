#include <locale.h>
#include <assert.h>
#include <stdint.h>

#include "../src/config.h"
#include "../src/map.h"
#include "../src/list.h"

int main() {
    //if (setlocale(LC_CTYPE, "C.UTF-8") == NULL) {
    //    fprintf(stderr, "can't set local C.UTF-8.\n");
    //    exit(EXIT_FAILURE);
    //}

    struct config_error error;
    struct config *c;
    struct config *config = config_load("./test_config.conf", &error);
    if (!config) {
        fprintf(stderr, "position: %ld, line: %ld, colume: %ld, %s\n", error.position, error.line, error.column, error.text);
        exit(EXIT_FAILURE);
    }
    config_dumps(config);

    /* object keys */
    c = config_object_get(config, "collect.exec.cpu_usage");
    assert(config_type(c) == CONFIG_OBJECT_TYPE);
    struct list *p, *keys = config_object_keys(c);
    for (p = keys; p != NULL; p = list_next(p)) {
        printf("%s\n", list_data(p));
        config_dumps(config_object_get(c, list_data(p)));
    }
    list_destroy(keys);

    /* array */
    c = config_object_get(config, "array");
    assert(config_type(c) == CONFIG_ARRAY_TYPE);
    size_t size = config_array_size(c);
    printf("list len: %d\n", size);
    for (size_t i = 0; i < size; i++) {
        config_dumps(config_array_get(c, i));
    }

    /* get path */
    c = config_object_get(config, "cache.max_count");
    assert(config_type(c) == CONFIG_INTEGER_TYPE);
    printf("cache.max_count: %ld\n", config_integer_value(c));

    /* string */
    c = config_object_get(config, "log_file");
    assert(config_type(c) == CONFIG_STRING_TYPE);
    printf("log_file: %s\n", config_string_value(c));

    /* integer */
    c = config_object_get(config, "port");
    assert(config_type(c) == CONFIG_INTEGER_TYPE);
    printf("port: %d\n", config_integer_value(c));

    /* double */
    c = config_object_get(config, "weight");
    assert(config_type(c) == CONFIG_DOUBLE_TYPE);
    printf("weight: %f\n", config_double_value(c));

    /* boolean */
    c = config_object_get(config, "cache_mem");
    assert(config_type(c) == CONFIG_BOOLEAN_TYPE);
    printf("cache_mem: %d\n", config_boolean_value(c));

    c = config_object_get(config, "log_size");
    assert(config_type(c) == CONFIG_SIZE_TYPE);
    printf("log_size(B): %lld\n", config_size_value(c, CONFIG_BYTE));
    printf("log_size(KB): %lld\n", config_size_value(c, CONFIG_KILO_BYTE));
    printf("log_size(MB): %lld\n", config_size_value(c, CONFIG_MEGA_BYTE));
    printf("log_size(GB): %lld\n", config_size_value(c, CONFIG_GIGA_BYTE));
    printf("log_size(TB): %lld\n", config_size_value(c, CONFIG_TERA_BYTE));
    printf("log_size(PB): %lld\n", config_size_value(c, CONFIG_PETA_BYTE));
    printf("log_size(EB): %lld\n", config_size_value(c, CONFIG_EXA_BYTE));
    printf("log_size(ZB): %lld\n", config_size_value(c, CONFIG_ZETTA_BYTE));
    printf("log_size(YB): %lld\n", config_size_value(c, CONFIG_YOTTA_BYTE));

    c = config_object_get(config, "file_size");
    assert(config_type(c) == CONFIG_SIZE_TYPE);
    printf("log_size(B): %lld\n", config_size_value(c, CONFIG_BYTE));
    printf("log_size(KB): %lld\n", config_size_value(c, CONFIG_KILO_BYTE));
    printf("log_size(MB): %lld\n", config_size_value(c, CONFIG_MEGA_BYTE));
    printf("log_size(GB): %lld\n", config_size_value(c, CONFIG_GIGA_BYTE));
    printf("log_size(TB): %lld\n", config_size_value(c, CONFIG_TERA_BYTE));
    printf("log_size(PB): %lld\n", config_size_value(c, CONFIG_PETA_BYTE));
    printf("log_size(EB): %lld\n", config_size_value(c, CONFIG_EXA_BYTE));
    printf("log_size(ZB): %lld\n", config_size_value(c, CONFIG_ZETTA_BYTE));
    printf("log_size(YB): %lld\n", config_size_value(c, CONFIG_YOTTA_BYTE));
 
    c = config_object_get(config, "interval");
    assert(config_type(c) == CONFIG_DURATION_TYPE);
    printf("interval(ns): %lld\n", config_duration_value(c, CONFIG_NANO_SECOND));
    printf("interval(us): %lld\n", config_duration_value(c, CONFIG_MICRO_SECOND));
    printf("interval(ms): %lld\n", config_duration_value(c, CONFIG_MILLI_SECOND));
    printf("interval(s): %lld\n", config_duration_value(c, CONFIG_SECOND));
    printf("interval(m): %lld\n", config_duration_value(c, CONFIG_MINUTE));
    printf("interval(h): %lld\n", config_duration_value(c, CONFIG_HOUR));
    printf("interval(d): %lld\n", config_duration_value(c, CONFIG_DAY));
 
    c = config_object_get(config, "period");
    assert(config_type(c) == CONFIG_DURATION_TYPE);
    printf("interval(ns): %lld\n", config_duration_value(c, CONFIG_NANO_SECOND));
    printf("interval(us): %lld\n", config_duration_value(c, CONFIG_MICRO_SECOND));
    printf("interval(ms): %lld\n", config_duration_value(c, CONFIG_MILLI_SECOND));
    printf("interval(s): %lld\n", config_duration_value(c, CONFIG_SECOND));
    printf("interval(m): %lld\n", config_duration_value(c, CONFIG_MINUTE));
    printf("interval(h): %lld\n", config_duration_value(c, CONFIG_HOUR));
    printf("interval(d): %lld\n", config_duration_value(c, CONFIG_DAY));
 
    //printf("a.s: %s\n", config_get_string(config, "a.s", "default"));
    //printf("a.i: %ld\n", config_get_integer(config, "a.i", 10));
    //printf("a.f: %lf\n", config_get_double(config, "a.f", 23.4));
    //printf("a.b: %d\n", config_get_boolean(config, "a.b", false));
    //struct duration d = config_get_duration(config, "a.d", 34, DURATION_SECOND);
    //printf("a.d: %ld, %d\n", d.value, d.unit);
    //struct config *object = config_get_object(config, "a.o");
    //config_dumps(object);
    //struct config *array = config_get_array(config, "a.a");
    //config_dumps(array);
    //struct list *p, *keys = config_get_object_keys(config);
    //printf("keys: ");
    //p = keys;
    //while(p) {
    //    printf("%s ", list_data(p));
    //    p = list_next(p);
    //}
    //list_destroy(keys);
    //printf("\n");
    config_destroy(config);
}
