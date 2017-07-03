#include <jansson.h>
#include "../config.h"

static void config_load_filesystem(const char *param) {
}

static void __attribute__((constructor)) config_register_filesystem() {
    config_register("filesystem", config_load_filesystem);
}

