#include <locale.h>
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
    struct config *config = config_load("./test_config.conf", &error);
    if (!config) {
        fprintf(stderr, "%s\n", error.text);
        exit(EXIT_FAILURE);
    }
    config_dumps(config);
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
    //config_destroy(config);
}
