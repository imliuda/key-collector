#include <locale.h>
#include <stdint.h>
#include "../src/config.h"
#include "../src/map.h"
#include "../src/list.h"

void config_dumps(struct config *config, int level) {
    if (config->type == CONFIG_OBJECT_TYPE  ) {
        printf("{\n");
        struct list *key, *keys = map_keys(config->value);
        key = keys;
        while (key = list_next(keys, key)) {
            for (int i = 0; i < level; i++) {
                printf("    ");
            }
            printf("%s: ", key->data);
            void *data;
            map_get(config->value, key->data, &data);
            config_dumps(data, level + 1);
        }
        for (int i = 1; i < level; i++) {
            printf("    ");
        }
        printf("}\n");
    } else if (config->type == CONFIG_ARRAY_TYPE) {
        struct list *p, *vs = config->value;
        p = vs;
        printf("[\n");
        while (p = list_next(vs, p)) {
            for (int i = 0; i < level; i++) {
                printf("    ");
            }
            config_dumps(p->data, level + 1);
        }
        for (int i = 1; i < level; i++) {
            printf("    ");
        }
        printf("]\n");
    } else if (config->type == CONFIG_STRING_TYPE) {
        printf("\"%s\"\n", config->value);
    } else if (config->type == CONFIG_INTEGER_TYPE) {
        printf("%ld\n", *(uint64_t *)(config->value));
    } else if (config->type == CONFIG_BOOLEAN_TYPE) {
        printf("%s\n", *(bool *)(config->value) ? "true" : "false");
    } else if (config->type == CONFIG_DURATION_TYPE) {
        struct duration *d = config->value;
        printf("%d", d->value);
        if (d->unit == DURATION_NANO_SECOND) {
            printf("ns");
        } else if (d->unit == DURATION_MICRO_SECOND) {
            printf("us");
        } else if (d->unit == DURATION_MILLI_SECOND) {
            printf("ms");
        } else if (d->unit == DURATION_SECOND) {
            printf("s");
        } else if (d->unit == DURATION_MINUTE) {
            printf("m");
        } else if (d->unit == DURATION_HOUR) {
            printf("h");
        } else if (d->unit == DURATION_DAY) {
            printf("d");
        }
        printf("\n");
    }
}

int main() {
    setlocale(LC_CTYPE, "C.UTF-8");

    struct config *config = config_load_file("../etc/osclt/osclt.conf");
    printf("load finished\n");
    config_dumps(config, 1);
}
