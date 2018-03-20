#include <locale.h>
#include "../src/config.h"
#include "../src/map.h"
#include "../src/list.h"

void config_dumps(struct config *config) {
    if (config->type == CONFIG_OBJECT_TYPE  ) {
        struct list *key, *keys = map_keys(config->value);
        key = keys;
        while (key = list_next(keys, key)) {
            printf("key: %s\n", key->data);
            void *data;
            map_get(config->value, key, &data);
            config_dumps(data);
        }
    } else if (config->type == CONFIG_ARRAY_TYPE) {
        struct list *p, *vs = config->value;
        while (p = list_next(vs, p)) {
            config_dumps(p);
        }
    } else if (config->type == CONFIG_STRING_TYPE) {
        printf("%s\n", config->value);
    }
}

int main() {
    setlocale(LC_CTYPE, "C.UTF-8");

    struct config *config = config_load("../etc/osclt/osclt.conf");
    printf("load finished\n");
    config_dumps(config);
}
