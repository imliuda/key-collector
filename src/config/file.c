#include <jansson.h>
#include "../config.h"

static void config_load_filesystem(const char *param) {
    json_error_t error;

    config = json_load_file(param, 0, &error);
    if (!config) {
        printf("error when opening config file: %s\n", param);
        return;
    }
}

static void __attribute__((constructor)) config_register_filesystem() {
    config_register("file", config_load_filesystem);
}
